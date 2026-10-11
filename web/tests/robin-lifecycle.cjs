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
  const media = {
    matches: reduce,
    addEventListener: (_event, fn) => { listener = fn; },
    removeEventListener: (_event, fn) => { assert.equal(listener, fn); listener = undefined; },
  };
  const angular = {
    Component: () => target => target,
    DestroyRef: {},
    inject: () => ({ onDestroy: fn => { destroy = fn; } }),
    afterNextRender: fn => { render = fn; },
    signal: value => {
      const read = () => value;
      read.set = next => { value = next; };
      return read;
    },
  };
  const context = {
    exports: {}, require: name => { assert.equal(name, '@angular/core'); return angular; },
    window: { matchMedia: () => media }, document: { hidden: false },
    setTimeout: (fn, delay) => { const id = ++nextId; timers.set(id, { fn, due: clock + delay }); return id; },
    clearTimeout: id => timers.delete(id),
  };
  vm.runInNewContext(compiled, context);
  const robin = new context.exports.RobinComponent();
  render();
  return {
    robin, timers,
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
f.tick(1000); assert.equal(f.robin.awake(), true);
f.tick(800); assert.equal(f.robin.blinking(), true);
f.tick(160); assert.equal(f.robin.blinking(), false);
f.tick(340); assert.equal(f.robin.look(), 'left');
f.tick(800); assert.equal(f.robin.look(), 'right');
f.robin.greet();
assert.equal(f.timers.size, 5); // Old greeting timers were cancelled.
assert.equal(f.robin.awake(), false);
assert.equal(f.robin.look(), null);
f.tick(4100);
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
