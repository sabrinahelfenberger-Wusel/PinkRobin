const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const ts = require('typescript');
const js = ts.transpileModule(fs.readFileSync('src/app/weather.ts', 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
const context = { exports: {} };
vm.runInNewContext(js, context);
const parse = context.exports.parseWeather;
const now = Date.parse('2026-10-18T12:30:00Z');
const valid = { source: 'MeteoSwiss', postalCode: '8001', pointId: '800100',
  forecastRunAt: '2026-10-18T12:00:00Z', retrievedAt: '2026-10-18T12:10:00Z',
  validFrom: '2026-10-18T12:00:00Z', validTo: '2026-10-18T13:00:00Z',
  temperatureC: 8.5, precipitationMm: 0, windSpeedKmh: 5, windGustKmh: 12 };
assert.equal(parse(valid, now), valid);
for (const body of [null, {}, 'error', {...valid, temperatureC: null}, {...valid, windSpeedKmh: NaN},
  {...valid, source: 'other'}, {...valid, postalCode: '123'}, {...valid, validTo: 'bad'},
  {...valid, validTo: '2026-10-18T12:30:00Z'}, {...valid, validFrom: '2026-10-18T13:00:00Z'}]) {
  assert.equal(parse(body, now), null);
}
console.log('Weather response validation, missing values and validity intervals passed.');
