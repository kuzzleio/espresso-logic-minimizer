'use strict';

const { test } = require('node:test');
const assert = require('node:assert');
const { minimize, Espresso } = require('..');

// (NOT (cond1 AND cond2) OR cond3) AND cond4 — the README example
const TABLE = [
  [[0, 0, 0, 0], [0]], [[0, 0, 0, 1], [1]], [[0, 0, 1, 0], [0]], [[0, 0, 1, 1], [1]],
  [[0, 1, 0, 0], [0]], [[0, 1, 0, 1], [1]], [[0, 1, 1, 0], [0]], [[0, 1, 1, 1], [1]],
  [[1, 0, 0, 0], [0]], [[1, 0, 0, 1], [1]], [[1, 0, 1, 0], [0]], [[1, 0, 1, 1], [1]],
  [[1, 1, 0, 0], [0]], [[1, 1, 0, 1], [0]], [[1, 1, 1, 0], [0]], [[1, 1, 1, 1], [1]],
];
const EXPECTED = ['--11 1', '0--1 1', '-0-1 1'];

test('minimize() reduces a PLA truth table to DNF', () => {
  const pla = ['.i 4', '.o 1', ...TABLE.map(([i, o]) => `${i.join('')} ${o.join('')}`), '.e'];

  assert.deepStrictEqual(minimize(pla).sort(), [...EXPECTED].sort());
});

test('Espresso reduces a progressively pushed truth table to DNF', () => {
  const espresso = new Espresso(4, 1);

  for (const [input, output] of TABLE) {
    espresso.push(input, output);
  }

  const result = espresso.minimize();

  assert.deepStrictEqual([...result].sort(), [...EXPECTED].sort());
  assert.strictEqual(espresso.minimize(), result, 'subsequent calls return the cached result');
});

test('minimize() returns an empty array for an empty table', () => {
  assert.deepStrictEqual(minimize([]), []);
});

test('minimize() rejects non-string content', () => {
  assert.throws(() => minimize('nope'), /expected an array/);
  assert.throws(() => minimize([1]), /Only strings are supported/);
});
