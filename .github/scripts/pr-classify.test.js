// Unit tests for Source classification, run against the copy INLINED in
// pr-board-sync.yml (the single source of truth), extracted at run time.
// No deps; run with: node .github/scripts/pr-classify.test.js
"use strict";

const assert = require("node:assert");
const { isInternalLogin, classifyAuthorSource } = require("./extract-workflow-js.js").load({
  workflow: ".github/workflows/pr-board-sync.yml",
  block: "classify",
});

const SRC = {
  sourceBot: "Bot",
  sourceInternal: "Internal",
  sourceCommunity: "Community",
};

const tests = [];
const test = (name, fn) => tests.push([name, fn]);

test("isInternalLogin: exact match", () => {
  assert.ok(isInternalLogin("alice", new Set(["alice", "bob"])));
  assert.ok(!isInternalLogin("carol", new Set(["alice", "bob"])));
});

test("isInternalLogin: case-insensitive", () => {
  assert.ok(isInternalLogin("Alice", new Set(["alice"])));
  assert.ok(isInternalLogin("alice", new Set(["Alice"])));
});

test("isInternalLogin: empty / missing", () => {
  assert.ok(!isInternalLogin("", new Set(["alice"])));
  assert.ok(!isInternalLogin(null, new Set(["alice"])));
  assert.ok(!isInternalLogin("alice", null));
  assert.ok(!isInternalLogin("alice", new Set()));
});

test("isInternalLogin: accepts Array members", () => {
  assert.ok(isInternalLogin("bob", ["alice", "bob"]));
});

test("classifyAuthorSource: bot short-circuit ignores membership", () => {
  assert.strictEqual(classifyAuthorSource({
    isBot: true, login: "alice", members: new Set(["alice"]), ...SRC,
  }), "Bot");
  assert.strictEqual(classifyAuthorSource({
    isBot: true, login: "outsider", members: new Set(), ...SRC,
  }), "Bot");
});

test("classifyAuthorSource: internal team member", () => {
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "alice", members: new Set(["alice", "bob"]), ...SRC,
  }), "Internal");
});

test("classifyAuthorSource: non-member is Community", () => {
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "outsider", members: new Set(["alice"]), ...SRC,
  }), "Community");
});

test("classifyAuthorSource: empty members fails safe to Community", () => {
  // Simulates listTeamMembers returning [] on a read error / unset team.
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "alice", members: new Set(), ...SRC,
  }), "Community");
});

(async () => {
  let passed = 0, failed = 0;
  for (const [name, fn] of tests) {
    try { await fn(); passed++; }
    catch (e) { failed++; console.error(`FAIL: ${name}\n  ${(e && e.stack) || e}`); }
  }
  console.log(`${passed} passed, ${failed} failed`);
  process.exit(failed ? 1 : 0);
})();
