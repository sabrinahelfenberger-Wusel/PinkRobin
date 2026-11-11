const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const ts = require('typescript');
const rx = require('rxjs');

const compilerOptions = {
    module: ts.ModuleKind.CommonJS,
    target: ts.ScriptTarget.ES2022,
    experimentalDecorators: true,
};

const compiled = ts.transpileModule(
    fs.readFileSync('src/app/robin-session.service.ts', 'utf8'),
    { compilerOptions },
).outputText;

const compiledOutput = ts.transpileModule(
    fs.readFileSync('src/app/core-output.ts', 'utf8'),
    { compilerOptions },
).outputText;

const outputContext = {
    exports: {},
    TextEncoder,
};
vm.runInNewContext(compiledOutput, outputContext);

function fixture() {
    const requests = [];
    const timers = new Map();
    const releases = [];
    const destroyed = new rx.Subject();
    const delays = [];

    let onDestroy;
    let nextTimer = 0;

    const clientToken = {};
    const destroyToken = {};

    const angular = {
        Injectable: () => (target) => target,
        DestroyRef: destroyToken,

        inject: (token) =>
            token === clientToken
                ? {
                      request: (method, url, options) => {
                          const subject = new rx.Subject();

                          requests.push({
                              method,
                              url,
                              options,
                              subject,
                          });

                          return subject;
                      },
                  }
                : {
                      onDestroy: (fn) => {
                          onDestroy = fn;
                      },
                  },

        signal: (value) => {
            const read = () => value;

            read.set = (next) => {
                value = next;
            };

            return read;
        },
    };

    const context = {
        exports: {},
        URL,
        TextEncoder,
        document: { baseURI: 'http://localhost/' },

        fetch: async (...args) => {
            releases.push(args);
        },

        setTimeout: (fn, delay) => {
            const id = ++nextTimer;
            delays.push(delay);
            timers.set(id, fn);
            return id;
        },

        clearTimeout: (id) => timers.delete(id),

        require: (name) =>
            name === './core-output'
                ? outputContext.exports
                : name === '@angular/core'
                  ? angular
                  : name === '@angular/common/http'
                    ? { HttpClient: clientToken }
                    : name === '@angular/core/rxjs-interop'
                      ? {
                            takeUntilDestroyed: () => rx.takeUntil(destroyed),
                        }
                      : rx,
    };

    vm.runInNewContext(compiled, context);

    return {
        service: new context.exports.RobinSessionService(),
        requests,
        delays,
        timers,
        releases,
        parse: context.exports.parseCoreState,

        reply(index, body) {
            assert.ok(requests[index], `Missing request ${index}`);
            requests[index].subject.next(body);
            requests[index].subject.complete();
        },

        tick() {
            const next = timers.entries().next().value;

            assert.ok(next, 'Expected a scheduled timer');

            const [id, fn] = next;
            timers.delete(id);
            fn();
        },

        destroy() {
            onDestroy();
            destroyed.next();
            destroyed.complete();
        },
    };
}

const flush = () => new Promise((resolve) => setImmediate(resolve));

const state = (sequence = 0, revision = 0, headContact = false) => ({
    mode: 'awake',
    mood: headContact ? 'happy' : 'curious',
    expression: headContact ? 'happy' : 'curious',
    headContact,
    timeMs: 7100,
    revision,
    sequence,
    transformation: 'none',
});

(async () => {
    const f = fixture();
    const connecting = f.service.connect(false);

    assert.equal(f.requests[0].url, 'api/v1/sessions');

    f.reply(0, {
        token: 'a'.repeat(64),
        state: state(),
    });

    await connecting;

    assert.equal(f.service.connection(), 'connected');

    // Loslassen wird hinter dem noch laufenden Start eingereiht.
    f.service.send('headStarted');
    f.service.send('headEnded');

    assert.equal(f.requests.length, 2);
    assert.equal(f.requests[1].options.body.sequence, 1);

    f.reply(1, state(1, 1, true));
    await flush();

    assert.equal(f.requests[2].url, 'api/v1/session/outputs/next');

    f.reply(2, null);
    await flush();
    f.tick();

    assert.equal(f.requests[3].options.body.event, 'headEnded');
    assert.equal(f.requests[3].options.body.sequence, 2);

    f.reply(3, state(2, 2));
    await flush();

    f.reply(4, null);
    await flush();

    assert.equal(f.service.state().headContact, false);

    // Eine laufende Zustandsabfrage darf den neuen Befehl
    // nicht überholen.
    f.tick();
    f.service.send('heartTouched');

    assert.equal(f.requests.length, 6);

    f.reply(5, state(2, 2));
    await flush();

    f.reply(6, null);
    await flush();
    f.tick();

    assert.equal(f.requests[7].options.body.sequence, 3);
    assert.equal(
        f.requests[7].options.headers['X-Robin-Session'],
        'a'.repeat(64),
    );

    f.reply(7, {
        ...state(3, 3),
        mood: 'happy',
        expression: 'happy',
    });

    await flush();

    f.reply(8, null);
    await flush();

    // Eine verspätete Antwort darf nach dem Zerstören
    // keinen Zustand mehr verändern.
    f.tick();
    f.destroy();
    await flush();

    assert.equal(f.timers.size, 0);
    assert.equal(f.releases.length, 1);

    f.reply(9, state(3, 99));
    await flush();

    assert.equal(f.service.state().revision, 3);

    // Eine Unterbrechung erhält Session und letzten Zustand.
    const broken = fixture();
    const start = broken.service.connect(true);

    broken.reply(0, {
        token: 'b'.repeat(64),
        state: state(0, 7),
    });

    await start;

    broken.tick();
    broken.requests[1].subject.error(new Error('offline'));
    await flush();

    assert.equal(broken.service.connection(), 'reconnecting');
    assert.equal(broken.service.state().revision, 7);
    assert.equal(broken.releases.length, 0);
    assert.equal(broken.timers.size, 1);

    // Während der Unterbrechung keine neuen Eingaben annehmen.
    broken.service.send('restart');
    assert.equal(broken.service.sendText('Hallo'), false);
    assert.equal(broken.requests.length, 2);

    // Auch der manuelle Verbindungsaufruf erzeugt keine neue Session.
    await broken.service.connect(true);
    assert.equal(broken.requests.length, 2);

    broken.tick();

    assert.equal(broken.requests[2].method, 'GET');
    assert.equal(broken.requests[2].url, 'api/v1/session');
    assert.equal(
        broken.requests[2].options.headers['X-Robin-Session'],
        'b'.repeat(64),
    );

    broken.reply(2, state(0, 8));
    await flush();

    assert.equal(broken.service.connection(), 'connected');
    assert.equal(broken.service.state().revision, 8);
    assert.equal(broken.releases.length, 0);

    broken.reply(3, null);
    await flush();

    // Eine tatsächlich abgelaufene Session beendet die Wiederverbindung.
    broken.tick();
    broken.requests[4].subject.error({ status: 404 });
    await flush();

    assert.equal(broken.service.connection(), 'local');
    assert.equal(broken.service.state(), null);
    assert.equal(broken.timers.size, 0);

    assert.throws(() => broken.parse({ ...state(), mode: 'made-up' }));
    assert.throws(() => broken.parse({ ...state(), timeMs: -1 }));

    broken.destroy();

    const delivery = fixture();
    const deliveryConnecting = delivery.service.connect(false);

    delivery.reply(0, {
        token: 'c'.repeat(64),
        state: state(),
    });
    await deliveryConnecting;

    delivery.service.send('weatherAsked');

    assert.equal(delivery.requests[1].options.body.event, 'weatherAsked');

    delivery.reply(1, state(1));
    await flush();

    delivery.reply(2, {
        sequence: '1',
        kind: 'weatherAnswer',
        weatherAnswer: 'rain',
        text: 'Die Prognose meldet Regen. Vergiss deinen Regenschutz nicht! ♡',
    });
    await flush();

    assert.equal(delivery.service.output().sequence, '1');
    assert.equal(delivery.service.output().weatherAnswer, 'rain');

    assert.equal(
        delivery.service.output().text,
        'Die Prognose meldet Regen. Vergiss deinen Regenschutz nicht! ♡',
    );

    // Eine fehlgeschlagene Bestätigung behält die Ausgabe.
    const failedAck = delivery.service.acknowledgeOutput('1');

    assert.equal(delivery.requests[3].url, 'api/v1/session/outputs/ack');
    assert.equal(delivery.requests[3].options.body.sequence, '1');

    delivery.requests[3].subject.error(new Error('offline'));

    assert.equal(await failedAck, false);
    assert.equal(delivery.service.output().sequence, '1');

    // Die Bestätigung kann erneut versucht werden.
    const successfulAck = delivery.service.acknowledgeOutput('1');

    delivery.reply(4, null);

    assert.equal(await successfulAck, true);
    assert.equal(delivery.service.output(), null);

    delivery.destroy();

    const textSession = fixture();
    const textConnecting = textSession.service.connect(false);

    textSession.reply(0, {
        token: 'd'.repeat(64),
        state: state(),
    });
    await textConnecting;

    assert.equal(textSession.service.sendText('   '), false);
    assert.equal(textSession.service.sendText('ä'.repeat(81)), false);

    // Abgewiesene Eingaben senden keinen Befehl.
    assert.equal(textSession.requests.length, 1);

    assert.equal(
        textSession.service.sendText('Wie wird das Wetter?'),
        true,
    );

    assert.equal(textSession.requests[1].options.body.sequence, 1);
    assert.equal(
        textSession.requests[1].options.body.event,
        'textReceived',
    );
    assert.equal(
        textSession.requests[1].options.body.text,
        'Wie wird das Wetter?',
    );

    textSession.reply(1, state(1));
    await flush();

    textSession.reply(2, {
        sequence: '1',
        kind: 'conversationReply',
        conversationReply: 'askPostalCode',
        text: 'Für welchen Ort möchtest du das Wetter wissen?',
    });
    await flush();

    assert.equal(
        textSession.service.output().conversationReply,
        'askPostalCode',
    );

    textSession.destroy();

    for (const alreadyAccepted of [true, false]) {
        const retry = fixture();
        const retryConnecting = retry.service.connect(false);

        retry.reply(0, {
            token: 'e'.repeat(64),
            state: state(),
        });

        await retryConnecting;

        assert.equal(retry.service.sendText('Hallo'), true);

        retry.requests[1].subject.error(new Error('offline'));
        await flush();

        assert.equal(retry.service.connection(), 'reconnecting');
        assert.equal(retry.releases.length, 0);

        retry.tick();

        // Vor jeder Wiederholung zuerst den Serverzustand lesen.
        assert.equal(retry.requests[2].method, 'GET');
        assert.equal(retry.requests[2].url, 'api/v1/session');

        retry.reply(2, state(alreadyAccepted ? 1 : 0));
        await flush();

        retry.reply(3, null);
        await flush();

        assert.equal(retry.service.connection(), 'connected');

        if (alreadyAccepted) {
            // Der Server hat den Text verarbeitet: nicht erneut senden.
            assert.equal(
                retry.requests.filter(
                    request => request.url === 'api/v1/session/events',
                ).length,
                1,
            );
        } else {
            retry.tick();

            // Nicht angenommen: denselben Text mit derselben Nummer senden.
            assert.equal(retry.requests[4].method, 'POST');
            assert.equal(retry.requests[4].options.body.sequence, 1);
            assert.equal(retry.requests[4].options.body.event, 'textReceived');
            assert.equal(retry.requests[4].options.body.text, 'Hallo');
            assert.equal(
                retry.requests[4].options.headers['X-Robin-Session'],
                'e'.repeat(64),
            );

            retry.reply(4, state(1));
            await flush();

            retry.reply(5, null);
            await flush();
        }

        retry.destroy();
    }

    for (const alreadyAccepted of [true, false]) {
        const contact = fixture();
        const contactConnecting = contact.service.connect(false);

        contact.reply(0, {
            token: 'f'.repeat(64),
            state: state(),
        });

        await contactConnecting;

        contact.service.send('headStarted');
        contact.requests[1].subject.error(new Error('offline'));
        await flush();

        contact.tick();

        contact.reply(
            2,
            state(
                alreadyAccepted ? 1 : 0,
                alreadyAccepted ? 1 : 0,
                alreadyAccepted,
            ),
        );
        await flush();

        contact.reply(3, null);
        await flush();
        contact.tick();

        const cancellation = contact.requests[4];

        assert.equal(cancellation.method, 'POST');
        assert.equal(cancellation.url, 'api/v1/session/events');
        assert.equal(
            cancellation.options.body.event,
            'interactionCancelled',
        );
        assert.equal(
            cancellation.options.body.sequence,
            alreadyAccepted ? 2 : 1,
        );
        assert.equal(
            cancellation.options.headers['X-Robin-Session'],
            'f'.repeat(64),
        );

        contact.reply(4, state(alreadyAccepted ? 2 : 1, 2, false));
        await flush();

        contact.reply(5, null);
        await flush();

        assert.equal(contact.service.state().headContact, false);
        assert.equal(contact.service.connection(), 'connected');
        assert.equal(contact.releases.length, 0);

        contact.destroy();
    }

    const resumed = fixture();
    const resumeConnecting = resumed.service.connect(false);

    resumed.reply(0, {
        token: '1'.repeat(64),
        state: state(0, 5),
    });

    await resumeConnecting;

    // Rückkehr zur Seite prüft die bestehende Session sofort.
    resumed.service.resume();

    assert.equal(resumed.requests.length, 2);
    assert.equal(resumed.requests[1].method, 'GET');
    assert.equal(resumed.requests[1].url, 'api/v1/session');
    assert.equal(
        resumed.requests[1].options.headers['X-Robin-Session'],
        '1'.repeat(64),
    );
    assert.equal(resumed.service.connection(), 'reconnecting');
    assert.equal(resumed.service.state().revision, 5);

    // Weitere Rückkehrsignale erzeugen keine parallele Anfrage.
    resumed.service.resume();
    assert.equal(resumed.requests.length, 2);

    resumed.reply(1, state(0, 6));
    await flush();

    resumed.reply(2, null);
    await flush();

    assert.equal(resumed.service.connection(), 'connected');
    assert.equal(resumed.service.state().revision, 6);
    assert.equal(resumed.releases.length, 0);
    assert.equal(
        resumed.requests.filter(
            request => request.url === 'api/v1/sessions',
        ).length,
        1,
    );

    resumed.destroy();

    const countAfterDestroy = resumed.requests.length;
    resumed.service.resume();

    assert.equal(resumed.requests.length, countAfterDestroy);
    assert.equal(resumed.timers.size, 0);

    const backoff = fixture();
    const backoffConnecting = backoff.service.connect(false);

    backoff.reply(0, {
        token: '2'.repeat(64),
        state: state(),
    });

    await backoffConnecting;

    for (const expectedDelay of [1000, 2000, 4000, 8000, 15000, 15000]) {
        backoff.tick();
        backoff.requests.at(-1).subject.error(new Error('offline'));
        await flush();

        assert.equal(backoff.delays.at(-1), expectedDelay);
        assert.equal(backoff.timers.size, 1);
        assert.equal(backoff.releases.length, 0);
    }

    // Eine Rückkehr zur Seite überspringt die aktuelle Wartezeit.
    const recoveryIndex = backoff.requests.length;
    backoff.service.resume();

    assert.equal(backoff.requests.length, recoveryIndex + 1);
    assert.equal(backoff.timers.size, 0);

    backoff.reply(recoveryIndex, state());
    await flush();

    backoff.reply(recoveryIndex + 1, null);
    await flush();

    assert.equal(backoff.service.connection(), 'connected');
    assert.equal(backoff.delays.at(-1), 250);

    // Nach erfolgreicher Verbindung beginnt die Wartefolge wieder bei 1 s.
    backoff.tick();
    backoff.requests.at(-1).subject.error(new Error('offline'));
    await flush();

    assert.equal(backoff.delays.at(-1), 1000);

    backoff.destroy();
    assert.equal(backoff.timers.size, 0);

    console.log(
        'Session ordering, early release, validation, fallback and destroy passed.',
    );
})().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
