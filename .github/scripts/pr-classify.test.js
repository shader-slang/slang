// Unit tests for Source classification, run against the copy INLINED in
// pr-board-sync.yml (the single source of truth), extracted at run time.
// No deps; run with: node .github/scripts/pr-classify.test.js
"use strict";

const assert = require("node:assert");
const {
  isInternalLogin,
  classifyAuthorSource,
  repoShortName,
  parseTeamScopeRepos,
  isSourceInternalFamilySlug,
  internalMembersForRepo,
} = require("./extract-workflow-js.js").load({
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

test("repoShortName", () => {
  assert.strictEqual(repoShortName("shader-slang/slangpy"), "slangpy");
  assert.strictEqual(repoShortName("slang-rhi"), "slang-rhi");
  assert.strictEqual(repoShortName(""), "");
});

test("parseTeamScopeRepos", () => {
  assert.deepStrictEqual(
    parseTeamScopeRepos(
      "Internal team members. Scope: [slangpy, slangpy-samples]"),
    ["slangpy", "slangpy-samples"]);
  assert.deepStrictEqual(
    parseTeamScopeRepos("Scope: [shader-slang/slang-rhi]"),
    ["slang-rhi"]);
  assert.deepStrictEqual(
    parseTeamScopeRepos(
      "Internal. Scope: [slangpy] Contact: alice, bob"),
    ["slangpy"]);
  assert.deepStrictEqual(
    parseTeamScopeRepos("Scope: slangpy, slangpy-samples"),
    []);
  assert.deepStrictEqual(parseTeamScopeRepos("no scope here"), []);
  assert.deepStrictEqual(parseTeamScopeRepos(""), []);
});

test("isSourceInternalFamilySlug", () => {
  assert.ok(isSourceInternalFamilySlug("source-internal", "source-internal"));
  assert.ok(isSourceInternalFamilySlug("source-internal", "source-internal-slangpy"));
  assert.ok(!isSourceInternalFamilySlug("source-internal", "source-internally"));
  assert.ok(!isSourceInternalFamilySlug("source-internal", "pr-owners"));
});

// The base team is the entry with repos === null (it covers every repo); every
// other entry covers only the repos its Scope: listed. members === null is a
// team whose roster could not be read.
const BASE = (...logins) => ({ repos: null, members: new Set(logins) });

test("internalMembersForRepo unions base and matching scoped teams", () => {
  const members = internalMembersForRepo("shader-slang/slangpy", [
    BASE("alice"),
    { repos: ["slangpy", "slangpy-samples"], members: new Set(["bob"]) },
    { repos: ["slang-rhi"], members: new Set(["carol"]) },
  ]);
  assert.ok(isInternalLogin("alice", members));
  assert.ok(isInternalLogin("bob", members));
  assert.ok(!isInternalLogin("carol", members));
});

test("internalMembersForRepo: unknown when a covering roster is unreadable", () => {
  assert.strictEqual(
    internalMembersForRepo("shader-slang/slangpy", [
      { repos: null, members: null },
    ]),
    null);
  assert.strictEqual(
    internalMembersForRepo("shader-slang/slangpy", [
      BASE("alice"),
      { repos: ["slangpy"], members: null },
    ]),
    null);
});

test("internalMembersForRepo: an unreadable roster for another repo is ignored", () => {
  const members = internalMembersForRepo("shader-slang/slangpy", [
    BASE("alice"),
    { repos: ["slang-rhi"], members: null },
  ]);
  assert.ok(isInternalLogin("alice", members));
  assert.ok(!isInternalLogin("carol", members));
});

test("internalMembersForRepo: unknown when the family itself is unreadable", () => {
  assert.strictEqual(internalMembersForRepo("shader-slang/slang", null), null);
});

test("internalMembersForRepo: no configured team is Community, not unknown", () => {
  const members = internalMembersForRepo("shader-slang/slang", []);
  assert.ok(members);
  assert.ok(!isInternalLogin("alice", members));
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

test("classifyAuthorSource: empty members is Community", () => {
  // A successful read of a family nobody is on (e.g. no team configured).
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "alice", members: new Set(), ...SRC,
  }), "Community");
});

test("classifyAuthorSource: unknown membership yields no Source", () => {
  // internalMembersForRepo returned null (unreadable roster). The caller must
  // leave Source unset rather than persist a transient error as Community.
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "alice", members: null, ...SRC,
  }), null);
  assert.strictEqual(classifyAuthorSource({
    isBot: false, login: "alice", members: undefined, ...SRC,
  }), null);
});

test("classifyAuthorSource: a bot stays Bot when membership is unknown", () => {
  assert.strictEqual(classifyAuthorSource({
    isBot: true, login: "dependabot", members: null, ...SRC,
  }), "Bot");
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
