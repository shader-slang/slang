// Unit tests for Source classification, run against the reconcile copy INLINED
// in pr-board-sync.yml and extracted at run time. The opened/reopened copy is
// extracted separately and required to contain byte-identical functions.
// No deps; run with: node .github/scripts/pr-classify.test.js
"use strict";

const assert = require("node:assert");
const extractor = require("./extract-workflow-js.js");
const workflow = ".github/workflows/pr-board-sync.yml";
const reconcile = extractor.load({
  workflow,
  block: "classify",
});
const onboarding = extractor.load({
  workflow,
  block: "classify-onboarding",
});
const {
  isInternalLogin,
  classifyAuthorSource,
  repoShortName,
  parseTeamScopeRepos,
  isSourceInternalFamilySlug,
  internalMembersForRepo,
} = reconcile;

const SRC = {
  sourceBot: "Bot",
  sourceInternal: "Internal",
  sourceCommunity: "Community",
};

const tests = [];
const test = (name, fn) => tests.push([name, fn]);

test("onboarding and reconcile helpers are byte-identical", () => {
  assert.deepStrictEqual(Object.keys(onboarding).sort(), Object.keys(reconcile).sort());
  for (const name of Object.keys(reconcile)) {
    assert.strictEqual(
      onboarding[name].toString(),
      reconcile[name].toString(),
      `${name} differs between onboarding and reconcile`);
  }
});

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

// Authoritative form is `Scope: [repo, ...]` (no space before the colon).
// Edges below pin what a team-description author can and cannot write.
test("parseTeamScopeRepos: case-insensitive label", () => {
  assert.deepStrictEqual(parseTeamScopeRepos("scope: [slangpy]"), ["slangpy"]);
  assert.deepStrictEqual(parseTeamScopeRepos("SCOPE: [slangpy]"), ["slangpy"]);
});

test("parseTeamScopeRepos: empty brackets yield no repos", () => {
  assert.deepStrictEqual(parseTeamScopeRepos("Scope: []"), []);
  assert.deepStrictEqual(parseTeamScopeRepos("Scope: [  ]"), []);
});

test("parseTeamScopeRepos: space before colon is not accepted", () => {
  // `\bScope:` requires the colon immediately after Scope; a typo like
  // "Scope : [...]" must not silently match (warn path covers missing Scope).
  assert.deepStrictEqual(parseTeamScopeRepos("Scope : [slangpy]"), []);
  assert.deepStrictEqual(parseTeamScopeRepos("Scope :[slangpy]"), []);
});

test("parseTeamScopeRepos: first Scope: wins", () => {
  assert.deepStrictEqual(
    parseTeamScopeRepos("Scope: [slangpy] Scope: [slang-rhi]"),
    ["slangpy"]);
});

test("parseTeamScopeRepos: whitespace inside brackets is fine", () => {
  assert.deepStrictEqual(
    parseTeamScopeRepos("Scope: [ slangpy , slang-rhi ]"),
    ["slangpy", "slang-rhi"]);
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

test("parseTeamScopeRepos: owner is stripped; short-name matching is cross-org", () => {
  // Scope: [otherorg/slangpy] is stored as "slangpy", so it matches
  // shader-slang/slangpy. Owners in Scope entries are never meaningful.
  assert.deepStrictEqual(
    parseTeamScopeRepos("Scope: [otherorg/slangpy]"),
    ["slangpy"]);
  const members = internalMembersForRepo("shader-slang/slangpy", [
    BASE("alice"),
    { repos: ["slangpy"], members: new Set(["bob"]) },
  ]);
  assert.ok(isInternalLogin("bob", members));
});

test("classifyAuthorSource: a bot stays Bot when membership is unknown", () => {
  assert.strictEqual(classifyAuthorSource({
    isBot: true, login: "dependabot", members: null, ...SRC,
  }), "Bot");
});

// --- team-roster cache (fake paginate, like pr-signal countingGh) -----------
const { createTeamReadCache } = extractor.load({
  workflow,
  block: "team-roster",
});

function roster({
  teams = [],
  members = {},
  failTeams = false,
  failMembers = {},
  maxAttempts = 2,
} = {}) {
  const warnings = [];
  let teamCalls = 0;
  const memberCalls = {};
  const cache = createTeamReadCache({
    maxAttempts,
    warn: (m) => warnings.push(m),
    paginateTeams: async () => {
      teamCalls++;
      if (failTeams) throw { status: 403 };
      return teams;
    },
    paginateMembers: async ({ org, team_slug }) => {
      const key = `${org}/${team_slug}`;
      memberCalls[key] = (memberCalls[key] || 0) + 1;
      if (failMembers[key]) throw { status: failMembers[key] };
      return (members[key] || []).map((login) => ({ login }));
    },
  });
  return { ...cache, warnings, get teamCalls() { return teamCalls; }, memberCalls };
}

test("tryListTeamMembers: bare slug is empty Set, not null", async () => {
  const r = roster();
  const set = await r.tryListTeamMembers("bare");
  assert.ok(set instanceof Set);
  assert.strictEqual(set.size, 0);
  assert.deepStrictEqual(r.memberCalls, {});
});

test("tryListTeamMembers: failed read is null", async () => {
  const r = roster({ failMembers: { "o/t": 403 } });
  assert.strictEqual(await r.tryListTeamMembers("o/t"), null);
  assert.ok(r.warnings.some((w) => w.includes("o/t")));
});

test("tryListTeamMembers: success is cached", async () => {
  const r = roster({ members: { "o/t": ["alice"] } });
  assert.ok(isInternalLogin("alice", await r.tryListTeamMembers("o/t")));
  assert.ok(isInternalLogin("alice", await r.tryListTeamMembers("o/t")));
  assert.strictEqual(r.memberCalls["o/t"], 1);
});

test("tryListTeamMembers: attempt budget is exactly maxAttempts", async () => {
  const r = roster({ failMembers: { "o/t": 500 }, maxAttempts: 2 });
  assert.strictEqual(await r.tryListTeamMembers("o/t"), null);
  assert.strictEqual(await r.tryListTeamMembers("o/t"), null);
  assert.strictEqual(await r.tryListTeamMembers("o/t"), null); // budget spent
  assert.strictEqual(r.memberCalls["o/t"], 2);
});

test("listTeamMembers: null becomes empty Set for owner pools", async () => {
  const r = roster({ failMembers: { "o/owners": 403 } });
  const set = await r.listTeamMembers("o/owners");
  assert.ok(set instanceof Set);
  assert.strictEqual(set.size, 0);
});

test("listOrgTeams: caches success and budgets failures", async () => {
  const ok = roster({ teams: [{ slug: "source-internal" }] });
  assert.strictEqual((await ok.listOrgTeams("o")).length, 1);
  assert.strictEqual((await ok.listOrgTeams("o")).length, 1);
  assert.strictEqual(ok.teamCalls, 1);

  const bad = roster({ failTeams: true, maxAttempts: 2 });
  assert.strictEqual(await bad.listOrgTeams("o"), null);
  assert.strictEqual(await bad.listOrgTeams("o"), null);
  assert.strictEqual(await bad.listOrgTeams("o"), null);
  assert.strictEqual(bad.teamCalls, 2);
});

test("warnOnce only emits once", () => {
  const r = roster();
  r.warnOnce("same");
  r.warnOnce("same");
  r.warnOnce("other");
  assert.deepStrictEqual(r.warnings, ["same", "other"]);
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
