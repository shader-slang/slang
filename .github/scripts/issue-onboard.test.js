// Unit tests for issue-onboard helpers inlined in issue-board-onboard.yml.
// No deps; run with: node .github/scripts/issue-onboard.test.js
"use strict";

const assert = require("node:assert");
const extractor = require("./extract-workflow-js.js");
const { currentIteration } = extractor.load({
  workflow: ".github/workflows/issue-board-onboard.yml",
  block: "current-iteration",
});

const tests = [];
const test = (name, fn) => tests.push([name, fn]);

const SPRINT_61 = {
  id: "02d25d3f",
  title: "Sprint 61",
  startDate: "2026-08-18",
  duration: 14,
};
const SPRINT_62 = {
  id: "a9562d51",
  title: "Sprint 62",
  startDate: "2026-09-01",
  duration: 14,
};

test("currentIteration: start date is inclusive", () => {
  assert.strictEqual(
    currentIteration([SPRINT_61, SPRINT_62], "2026-08-18T00:00:00Z").title,
    "Sprint 61",
  );
});

test("currentIteration: last day of duration is still in range", () => {
  // 18 Aug + 14 days = 1 Sep 00:00 exclusive, so 31 Aug is still Sprint 61.
  assert.strictEqual(
    currentIteration([SPRINT_61, SPRINT_62], "2026-08-31T23:59:59Z").title,
    "Sprint 61",
  );
});

test("currentIteration: duration end is exclusive", () => {
  assert.strictEqual(
    currentIteration([SPRINT_61, SPRINT_62], "2026-09-01T00:00:00Z").title,
    "Sprint 62",
  );
});

test("currentIteration: empty / missing yields null", () => {
  assert.strictEqual(currentIteration([], "2026-08-31T12:00:00Z"), null);
  assert.strictEqual(currentIteration(null, "2026-08-31T12:00:00Z"), null);
  assert.strictEqual(currentIteration(undefined, "2026-08-31T12:00:00Z"), null);
});

test("currentIteration: no covering window yields null", () => {
  assert.strictEqual(
    currentIteration([SPRINT_61], "2026-08-01T00:00:00Z"),
    null,
  );
});

test("currentIteration: skips malformed entries", () => {
  assert.strictEqual(
    currentIteration(
      [{ title: "bad" }, SPRINT_61],
      "2026-08-20T12:00:00Z",
    ).title,
    "Sprint 61",
  );
});

test("currentIteration: accepts a Date", () => {
  assert.strictEqual(
    currentIteration([SPRINT_61], new Date("2026-08-25T12:00:00Z")).title,
    "Sprint 61",
  );
});

(async () => {
  let passed = 0,
    failed = 0;
  for (const [name, fn] of tests) {
    try {
      await fn();
      passed++;
    } catch (e) {
      failed++;
      console.error(`FAIL: ${name}\n  ${(e && e.stack) || e}`);
    }
  }
  console.log(`${passed} passed, ${failed} failed`);
  process.exit(failed ? 1 : 0);
})();
