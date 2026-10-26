const assert = require('node:assert/strict'), fs = require('node:fs'), vm = require('node:vm');
const ts = require('typescript'), rx = require('rxjs');
const compiled = ts.transpileModule(fs.readFileSync('src/app/robin-session.service.ts', 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022, experimentalDecorators: true },
}).outputText;
function fixture() {
  const requests = [], timers = new Map(), releases = [], destroyed = new rx.Subject();
  let onDestroy, nextTimer = 0;
  const clientToken = {}, destroyToken = {};
  const angular = {
    Injectable: () => target => target, DestroyRef: destroyToken,
    inject: token => token === clientToken ? { request: (method, url, options) => {
      const subject = new rx.Subject(); requests.push({ method, url, options, subject }); return subject;
    } } : { onDestroy: fn => { onDestroy = fn; } },
    signal: value => { const read = () => value; read.set = next => { value = next; }; return read; },
  };
  const context = { exports: {}, URL, document: { baseURI: 'http://localhost/' },
    fetch: async (...args) => { releases.push(args); },
    setTimeout: fn => { const id = ++nextTimer; timers.set(id, fn); return id; },
    clearTimeout: id => timers.delete(id),
    require: name => name === '@angular/core' ? angular : name === '@angular/common/http' ? { HttpClient: clientToken } :
      name === '@angular/core/rxjs-interop' ? { takeUntilDestroyed: () => rx.takeUntil(destroyed) } : rx,
  };
  vm.runInNewContext(compiled, context);
  return { service: new context.exports.RobinSessionService(), requests, timers, releases,
    parse: context.exports.parseCoreState,
    reply(index, body) { requests[index].subject.next(body); requests[index].subject.complete(); },
    tick() { const [id, fn] = timers.entries().next().value; timers.delete(id); fn(); },
    destroy() { onDestroy(); destroyed.next(); destroyed.complete(); },
  };
}
const flush = () => new Promise(resolve => setImmediate(resolve));
const state = (sequence = 0, revision = 0, headContact = false) => ({ mode: 'awake',
  mood: headContact ? 'happy' : 'curious', expression: headContact ? 'happy' : 'curious',
  headContact, timeMs: 7100, revision, sequence, transformation: 'none' });
(async () => {
  const f = fixture(), connecting = f.service.connect(false);
  assert.equal(f.requests[0].url, 'api/v1/sessions');
  f.reply(0, { token: 'a'.repeat(64), state: state() }); await connecting;
  assert.equal(f.service.connection(), 'connected');
  f.service.send('headStarted'); f.service.send('headEnded');
  assert.equal(f.requests.length, 2);
  assert.equal(f.requests[1].options.body.sequence, 1);
  f.reply(1, state(1, 1, true)); await flush(); f.tick();
  assert.equal(f.requests[2].options.body.event, 'headEnded');
  assert.equal(f.requests[2].options.body.sequence, 2);
  f.reply(2, state(2, 2)); await flush();
  assert.equal(f.service.state().headContact, false);
  f.tick(); f.service.send('heartTouched');
  assert.equal(f.requests.length, 4);
  f.reply(3, state(2, 2)); await flush(); f.tick();
  assert.equal(f.requests[4].options.body.sequence, 3);
  assert.equal(f.requests[4].options.headers['X-Robin-Session'], 'a'.repeat(64));
  f.reply(4, { ...state(3, 3), mood: 'happy', expression: 'happy' }); await flush();
  f.tick(); f.destroy(); await flush();
  assert.equal(f.timers.size, 0); assert.equal(f.releases.length, 1);
  f.reply(5, state(3, 99)); await flush();
  assert.equal(f.service.state().revision, 3);
  const broken = fixture(), start = broken.service.connect(true);
  broken.reply(0, { token: 'b'.repeat(64), state: state() }); await start;
  broken.tick(); broken.requests[1].subject.error(new Error('offline')); await flush();
  assert.equal(broken.service.connection(), 'local'); assert.equal(broken.service.state(), null);
  assert.equal(broken.timers.size, 0);
  broken.service.send('restart'); assert.equal(broken.requests.length, 2);
  assert.throws(() => broken.parse({ ...state(), mode: 'made-up' }));
  assert.throws(() => broken.parse({ ...state(), timeMs: -1 }));
  broken.destroy();
  console.log('Session ordering, early release, validation, fallback and destroy passed.');
})().catch(error => { console.error(error); process.exitCode = 1; });
