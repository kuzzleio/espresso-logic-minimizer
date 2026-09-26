'use strict';
// Usage: node test/parity.js <reference package dir> <candidate package dir> [runs]
// CI runs it against the last NAN release (2.2.0) to prove the N-API port
// returns exactly the same results.
//
// Differential test: minimize the same seeded random truth tables, through
// both entry points (`minimize` and the `Espresso` class koncorde uses), with
// each build, and compare every result.
const path = require('path');

const [refPath, candidatePath, n = '2000'] = process.argv.slice(2);
const RUNS = Number(n);

function run(modPath) {
  const { minimize, Espresso } = require(path.resolve(modPath));
  let seed = 42;
  const rnd = () => (seed = (seed * 1103515245 + 12345) % 2147483648) / 2147483648;
  const bits = (k) => Array.from({ length: k }, () => (rnd() < 0.5 ? 1 : 0));
  const out = [];

  for (let r = 0; r < RUNS; r++) {
    const inputs = 1 + Math.floor(rnd() * 8);
    const outputs = 1 + Math.floor(rnd() * 3);
    const rows = Math.floor(rnd() * (1 << inputs));

    const espresso = new Espresso(inputs, outputs);
    const lines = [`.i ${inputs}`, `.o ${outputs}`];

    for (let i = 0; i < rows; i++) {
      const input = bits(inputs);
      const output = bits(outputs);
      espresso.push(input, output);
      lines.push(`${input.join('')} ${output.join('')}`);
    }

    out.push(espresso.minimize(), minimize(lines));
  }

  out.push(minimize([]), minimize(['.i 2', '.o 1', '1- 1', '-1 1']));
  return JSON.stringify(out);
}

const a = run(refPath);
const b = run(candidatePath);

if (a !== b) {
  const A = JSON.parse(a);
  const B = JSON.parse(b);
  const i = A.findIndex((x, k) => JSON.stringify(x) !== JSON.stringify(B[k]));
  console.log(`MISMATCH at result ${i}:\n ref=${JSON.stringify(A[i])}\n new=${JSON.stringify(B[i])}`);
  process.exit(1);
}

const terms = JSON.parse(a).reduce((s, r) => s + r.length, 0);
console.log(
  `PARITY OK: ${JSON.parse(a).length} minimizations identical (${terms} terms), node ${process.version} ${process.platform}-${process.arch}`,
);
