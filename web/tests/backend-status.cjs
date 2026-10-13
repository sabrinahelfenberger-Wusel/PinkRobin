const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const ts = require('typescript');
const js = ts.transpileModule(fs.readFileSync('src/app/backend-status.ts', 'utf8'), {
  compilerOptions: {module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022},
}).outputText;
const context = {exports: {}};
vm.runInNewContext(js, context);
const parse = context.exports.parseBackendStatus;
const valid = {service: 'PinkRobin.Api', apiVersion: '1.0', core: {connected: false, implementation: 'not-connected'}};
assert.equal(parse(valid).core.connected, false);
assert.equal(parse({...valid, core: {connected: true, implementation: 'native'}}).core.connected, true);
for (const body of [null, '<html>error</html>', {}, {...valid, apiVersion: '2.0'}, {...valid, service: 'other'}, {...valid, core: {}}, {...valid, core: {connected: 'false', implementation: 'native'}}]) {
  assert.equal(parse(body), null);
}
console.log('Backend status validation and Core connection distinction passed.');
