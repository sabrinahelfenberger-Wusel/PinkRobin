// Exercise the component's timer lifecycle with a deterministic clock.
// Angular's template and type checking are covered by `npm run build`.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const ts = require('typescript');
const source = fs.readFileSync('src/app/robin.component.ts', 'utf8');
const compiled = ts.transpileModule(source, {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022, experimentalDecorators: true },
}).outputText;

function fixture(reduce = false) {
  let clock = 0, nextId = 0, destroy, render, listener;
  const timers = new Map();
  const effects = [], sent = [];
  let coreState = null, connection = 'local';
  const session = { connection: () => connection, state: () => coreState,
    connect: async () => {}, send: event => sent.push(event) };
  const media = {
    matches: reduce,
    addEventListener: (_event, fn) => { listener = fn; },
    removeEventListener: (_event, fn) => { assert.equal(listener, fn); listener = undefined; },
  };
  const angular = {
    Component: () => target => target,
    DestroyRef: {},
    effect: fn => effects.push(fn),
    inject: token => token === angular.DestroyRef ? ({ onDestroy: fn => { destroy = fn; } }) : session,
    afterNextRender: fn => { render = fn; },
    computed: read => read,
    signal: value => {
      const read = () => value;
      read.set = next => { value = next; };
      return read;
    },
  };
  const context = {
    exports: {}, require: name => name === '@angular/core' ? angular : { RobinSessionService: {} },
    window: { matchMedia: () => media }, document: { hidden: false },
    setTimeout: (fn, delay) => { const id = ++nextId; timers.set(id, { fn, due: clock + delay }); return id; },
    clearTimeout: id => timers.delete(id),
  };
  vm.runInNewContext(compiled, context);
  const robin = new context.exports.RobinComponent();
  render();
  return {
    robin, timers, sent,
    core(state) { coreState = state; connection = state ? 'connected' : 'local'; effects.forEach(fn => fn()); },
    tick(ms) {
      const end = clock + ms;
      while (true) {
        const next = [...timers].filter(([, t]) => t.due <= end).sort((a, b) => a[1].due - b[1].due)[0];
        if (!next) break;
        clock = next[1].due; timers.delete(next[0]); next[1].fn();
      }
      clock = end;
    },
    motion(value) { media.matches = value; listener(); },
    destroy() { destroy(); assert.equal(listener, undefined); },
  };
}

const f = fixture();
assert.equal(f.robin.awake(), false);
f.tick(3999); assert.equal(f.robin.awake(), false);
f.tick(1); assert.equal(f.robin.awake(), true);
f.tick(800); assert.equal(f.robin.blinking(), true);
f.tick(160); assert.equal(f.robin.blinking(), false);
f.tick(340); assert.equal(f.robin.look(), 'left');
f.tick(800); assert.equal(f.robin.look(), 'right');
f.robin.greet();
assert.equal(f.timers.size, 5); // Old greeting timers were cancelled.
assert.equal(f.robin.awake(), false);
assert.equal(f.robin.look(), null);
f.tick(7100);
assert.equal(f.robin.speech(), 'Hallo, ich bin Robin! Schön, dass du da bist. ♡');
assert.equal(f.robin.look(), null);
assert.equal(f.timers.size, 1); // One idle loop.
f.motion(true);
assert.equal(f.timers.size, 0);
assert.equal(f.robin.awake(), true);
f.motion(false);
assert.equal(f.timers.size, 5);
f.destroy();
assert.equal(f.timers.size, 0);
f.robin.greet(); assert.equal(f.timers.size, 0);
f.tick(10000);

const reduced = fixture(true);
assert.equal(reduced.robin.awake(), true);
assert.equal(reduced.robin.state(), 'Robin ist wach und neugierig');
assert.equal(reduced.timers.size, 0);
reduced.robin.greet(); assert.equal(reduced.timers.size, 0);
reduced.destroy();
console.log('Greeting, restart, reduced motion, preference changes and destroy cleanup passed.');

const touched = fixture();
touched.tick(5300);
touched.robin.touchHeart();
assert.equal(touched.robin.awake(), true);
assert.equal(touched.robin.look(), null);
assert.equal(touched.robin.blinking(), true);
assert.equal(touched.robin.heartGlowing(), true);
touched.tick(160);
assert.equal(touched.robin.blinking(), false);
touched.tick(690);
assert.equal(touched.robin.heartGlowing(), false);
assert.equal(touched.robin.mood(), 'happy');
touched.tick(5149);
assert.equal(touched.robin.mood(), 'happy');
touched.tick(1);
assert.equal(touched.robin.mood(), 'curious');
assert.equal(touched.robin.speech(), 'Was entdecken wir als Nächstes?');
for (let i = 0; i < 20; i++) touched.robin.touchHeart();
assert.equal(touched.timers.size, 4);
touched.robin.greet();
assert.equal(touched.robin.heartGlowing(), false);
assert.equal(touched.robin.blinking(), false);
assert.equal(touched.timers.size, 5);
touched.robin.touchHeart();
touched.destroy();
assert.equal(touched.timers.size, 0);
touched.robin.touchHeart();
assert.equal(touched.timers.size, 0);
const quietTouch = fixture(true);
quietTouch.robin.touchHeart();
assert.equal(quietTouch.robin.speech(), 'Das tut gut. Schön, dass du da bist. ♡');
assert.equal(quietTouch.robin.blinking(), false);
assert.equal(quietTouch.robin.heartGlowing(), false);
assert.equal(quietTouch.timers.size, 1);
assert.equal(quietTouch.robin.mood(), 'happy');
quietTouch.tick(6000);
assert.equal(quietTouch.robin.mood(), 'curious');
assert.equal(quietTouch.timers.size, 0);
quietTouch.motion(false);
quietTouch.robin.touchHeart();
quietTouch.motion(true);
assert.equal(quietTouch.robin.heartGlowing(), false);
assert.equal(quietTouch.robin.blinking(), false);
assert.equal(quietTouch.timers.size, 0);
quietTouch.destroy();
console.log('Heart response, interrupted greeting, repeated touches and reduced motion passed.');

const refreshed = fixture(true);
refreshed.robin.touchHeart();
refreshed.tick(5000);
refreshed.robin.touchHeart();
refreshed.tick(1000);
assert.equal(refreshed.robin.mood(), 'happy');
refreshed.tick(5000);
assert.equal(refreshed.robin.mood(), 'curious');
refreshed.robin.touchHeart();
refreshed.robin.greet();
assert.equal(refreshed.robin.mood(), 'curious');
refreshed.tick(6000);
assert.equal(refreshed.robin.speech(), 'Hallo, ich bin Robin! Schön, dass du da bist. ♡');
refreshed.destroy();
console.log('Mood transitions, six-second happiness and renewed touch duration passed.');

const pet = fixture(true);
pet.robin.startPetting();
assert.equal(pet.robin.mood(), 'happy');
pet.tick(4999);
pet.robin.startPetting(); // Continued strokes/key repeats do not restart the clock.
assert.equal(pet.robin.mood(), 'happy');
pet.tick(1);
assert.equal(pet.robin.mood(), 'affectionate');
pet.robin.stopPetting();
assert.equal(pet.robin.mood(), 'curious');
assert.equal(pet.timers.size, 0);
pet.robin.startPetting();
pet.tick(2000);
pet.robin.stopPetting();
pet.tick(5000);
assert.equal(pet.robin.mood(), 'curious');
pet.robin.startPetting();
pet.robin.touchHeart();
pet.tick(5000);
assert.equal(pet.robin.mood(), 'happy');
pet.robin.startPetting();
pet.robin.greet();
pet.tick(5000);
assert.equal(pet.robin.mood(), 'curious');
pet.robin.startPetting();
pet.destroy();
assert.equal(pet.timers.size, 0);
console.log('Petting, exact five-second heart eyes, early release, interruption and cleanup passed.');

const online = fixture(true);
online.core({ mode: 'awake', mood: 'curious', headContact: false, revision: 1 });
online.robin.startPetting();
assert.equal(online.robin.mood(), 'curious'); // No optimistic local state decision.
online.robin.stopPetting(); // Release before the server acknowledges the press.
assert.deepEqual(online.sent, ['headStarted', 'headEnded']);
online.core({ mode: 'awake', mood: 'happy', headContact: true, revision: 2 });
assert.equal(online.robin.mood(), 'happy');
online.tick(5000);
assert.equal(online.robin.mood(), 'happy'); // Only the Core may decide the threshold.
online.core({ mode: 'awake', mood: 'affectionate', headContact: true, revision: 3 });
assert.equal(online.robin.mood(), 'affectionate');
online.core({ mode: 'awake', mood: 'curious', headContact: false, revision: 4 });
online.robin.touchHeart();
assert.equal(online.sent.at(-1), 'heartTouched');
online.core({ mode: 'awake', mood: 'happy', headContact: false, revision: 5 });
online.tick(6000);
assert.equal(online.robin.mood(), 'happy');
online.robin.greet();
assert.equal(online.sent.at(-1), 'restart');
online.core(null);
assert.equal(online.robin.mood(), 'curious');
online.destroy();
console.log('Server-owned expressions, early release and local fallback passed.');

const rapid = fixture(true);
rapid.core({ mode: 'awake', mood: 'happy', headContact: true, revision: 1 });
// A new press must be accepted even while an older release is awaiting acknowledgement.
rapid.robin.startPetting();
rapid.robin.stopPetting();
rapid.robin.startPetting();
assert.deepEqual(rapid.sent, ['headStarted', 'headEnded', 'headStarted']);
rapid.destroy();
console.log('Rapid release and renewed press before acknowledgement passed.');
