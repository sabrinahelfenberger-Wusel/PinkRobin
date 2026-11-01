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

const outputContext = { exports: {} };
vm.runInNewContext(compiledOutput, outputContext);

function fixture() {
    const requests = [];
    const timers = new Map();
    const releases = [];
    const destroyed = new rx.Subject();

    let onDestroy;
    let nextTimer = 0;

    const clientToken = {};
    const destroyToken = {};

    const angular = {
        Injectable: () => target => target,
        DestroyRef: destroyToken,

        inject: token =>
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
                    onDestroy: fn => {
                        onDestroy = fn;
                    },
                },

        signal: value => {
            const read = () => value;

            read.set = next => {
                value = next;
            };

            return read;
        },
    };

    const context = {
        exports: {},
        URL,
        document: { baseURI: 'http://localhost/' },

        fetch: async (...args) => {
            releases.push(args);
        },

        setTimeout: fn => {
            const id = ++nextTimer;
            timers.set(id, fn);
            return id;
        },

        clearTimeout: id => timers.delete(id),

        require: name =>
            name === './core-output'
                ? outputContext.exports
                : name === '@angular/core'
                    ? angular
                    : name === '@angular/common/http'
                        ? { HttpClient: clientToken }
                        : name === '@angular/core/rxjs-interop'
                            ? {
                                takeUntilDestroyed: () =>
                                    rx.takeUntil(destroyed),
                            }
                            : rx,
    };

    vm.runInNewContext(compiled, context);

    return {
        service: new context.exports.RobinSessionService(),
        requests,
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

const flush = () =>
    new Promise(resolve => setImmediate(resolve));

const state = (
    sequence = 0,
    revision = 0,
    headContact = false,
) => ({
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

    assert.equal(
        f.requests[2].url,
        'api/v1/session/outputs/next',
    );

    f.reply(2, null);
    await flush();
    f.tick();

    assert.equal(
        f.requests[3].options.body.event,
        'headEnded',
    );
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

    // Verbindungsfehler führen zur lokalen Darstellung.
    const broken = fixture();
    const start = broken.service.connect(true);

    broken.reply(0, {
        token: 'b'.repeat(64),
        state: state(),
    });

    await start;

    broken.tick();
    broken.requests[1].subject.error(new Error('offline'));
    await flush();

    assert.equal(broken.service.connection(), 'local');
    assert.equal(broken.service.state(), null);
    assert.equal(broken.timers.size, 0);

    broken.service.send('restart');
    assert.equal(broken.requests.length, 2);

    assert.throws(() =>
        broken.parse({ ...state(), mode: 'made-up' }),
    );

    assert.throws(() =>
        broken.parse({ ...state(), timeMs: -1 }),
    );

    broken.destroy();

    const delivery = fixture();
    const deliveryConnecting = delivery.service.connect(false);

    delivery.reply(0, {
        token: 'c'.repeat(64),
        state: state(),
    });
    await deliveryConnecting;

    delivery.service.send('weatherAsked');

    assert.equal(
        delivery.requests[1].options.body.event,
        'weatherAsked',
    );

    delivery.reply(1, state(1));
    await flush();

    delivery.reply(2, {
        sequence: '1',
        kind: 'weatherAnswer',
        weatherAnswer: 'rain',
    });
    await flush();

    assert.equal(delivery.service.output().sequence, '1');
    assert.equal(delivery.service.output().weatherAnswer, 'rain');

    // Eine fehlgeschlagene Bestätigung behält die Ausgabe.
    const failedAck = delivery.service.acknowledgeOutput('1');

    assert.equal(
        delivery.requests[3].url,
        'api/v1/session/outputs/ack',
    );
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

    console.log(
        'Session ordering, early release, validation, fallback and destroy passed.',
    );
})().catch(error => {
    console.error(error);
    process.exitCode = 1;
});