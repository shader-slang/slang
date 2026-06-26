// Unit tests for the committer-signal ranking (the JS port of pr_signal.py),
// run against the copy INLINED in pr-board-sync.yml (the single source of
// truth), extracted at run time. No deps; run with:
//   node .github/scripts/pr-signal.test.js
// Mirrors the pure-helper and cache tests in the slang-skills test_pr_sweep.py.
"use strict";

const assert = require("node:assert");
const s = require("./extract-workflow-js.js").load();

const tests = [];
const test = (name, fn) => tests.push([name, fn]);
const approx = (a, b) => assert.ok(Math.abs(a - b) < 1e-9, `${a} !~= ${b}`);

// --- classification --------------------------------------------------------
test("classify_is_bot variants", () => {
  const bots = ["nv-slang-bot", "slang-coworker-nanoclaw"];
  assert.ok(s.classifyIsBot("nv-slang-bot", bots));
  assert.ok(s.classifyIsBot("nv-slang-bot[bot]", bots));
  assert.ok(s.classifyIsBot("app/nv-slang-bot", bots));
  assert.ok(s.classifyIsBot("slang-coworker-nanoclaw[bot]", bots));
  assert.ok(!s.classifyIsBot("some-human", bots));
});

test("is_bot_login", () => {
  const bots = ["nv-slang-bot"];
  assert.ok(s.isBotLogin("github-actions[bot]", bots));
  assert.ok(s.isBotLogin("nv-slang-bot", bots));
  assert.ok(s.isBotLogin("", bots));
  assert.ok(!s.isBotLogin("alice", bots));
});

// --- attribution -----------------------------------------------------------
const BOTS = ["nv-slang-bot"];
test("attribute: human author credited", () =>
  assert.strictEqual(s.attributeCommit("alice", null, "author", BOTS), "alice"));
test("attribute: author-self uses approver", () =>
  assert.strictEqual(s.attributeCommit("author", "carol", "author", BOTS), "carol"));
test("attribute: author-self no approver dropped", () =>
  assert.strictEqual(s.attributeCommit("author", null, "author", BOTS), null));
test("attribute: author-self approver-is-author dropped", () =>
  assert.strictEqual(s.attributeCommit("author", "author", "author", BOTS), null));
test("attribute: bot author uses approver", () =>
  assert.strictEqual(s.attributeCommit("nv-slang-bot", "carol", "author", BOTS), "carol"));
test("attribute: unmapped author uses approver", () =>
  assert.strictEqual(s.attributeCommit("", "carol", "author", BOTS), "carol"));
test("attribute: bot author no approver dropped", () =>
  assert.strictEqual(s.attributeCommit("nv-slang-bot", null, "author", BOTS), null));
test("attribute: bot author approver-is-pr-author dropped", () =>
  assert.strictEqual(s.attributeCommit("nv-slang-bot", "author", "author", BOTS), null));
test("attribute: bot author approver-is-bot dropped", () =>
  assert.strictEqual(s.attributeCommit("nv-slang-bot", "nv-slang-bot", "author", BOTS), null));

// --- weighting -------------------------------------------------------------
const TABLE = [["source/slang/**", 3.0], ["source/**", 2.0], ["tests/**", 1.0]];
test("multiplier most-specific wins", () => {
  assert.strictEqual(s.matchFileMultiplier("source/slang/x.cpp", TABLE, 1.0), 3.0);
  assert.strictEqual(s.matchFileMultiplier("source/core/x.cpp", TABLE, 1.0), 2.0);
  assert.strictEqual(s.matchFileMultiplier("tests/x.slang", TABLE, 1.0), 1.0);
  assert.strictEqual(s.matchFileMultiplier("README.md", TABLE, 1.0), 1.0);
});

test("per_file_signals", () => {
  const sig = s.perFileSignals({ "source/slang/a.cpp": 10.0, "tests/b.slang": 10.0 }, TABLE, 1.0);
  assert.deepStrictEqual(sig, { "source/slang/a.cpp": 30.0, "tests/b.slang": 10.0 });
});

test("top_k_weights normalizes", () => {
  const w = s.topKWeights({ a: 30.0, b: 10.0 }, 10);
  approx(w.a, 0.75); approx(w.b, 0.25);
});
test("top_k_weights limits", () => {
  const w = s.topKWeights({ a: 3.0, b: 2.0, c: 1.0 }, 2);
  assert.deepStrictEqual(new Set(Object.keys(w)), new Set(["a", "b"]));
});
test("top_k_weights empty when no signal", () =>
  assert.deepStrictEqual(s.topKWeights({ a: 0.0 }, 5), {}));

test("overall_signal sums weight*loc", () => {
  const overall = s.overallSignal(
    { f1: 0.75, f2: 0.25 }, { f1: { alice: 40.0, bob: 10.0 }, f2: { bob: 20.0 } });
  approx(overall.alice, 30.0);
  approx(overall.bob, 0.75 * 10 + 0.25 * 20);
});

test("rank_logins descending", () =>
  assert.deepStrictEqual(s.rankLogins({ alice: 30.0, bob: 12.5 }), ["alice", "bob"]));

// --- tiebreak bookkeeping --------------------------------------------------
test("needs_tiebreak close", () =>
  assert.ok(s.needsTiebreak({ a: 10.0, b: 8.0 }, ["a", "b"], 1.5)));
test("no tiebreak clear winner", () =>
  assert.ok(!s.needsTiebreak({ a: 20.0, b: 8.0 }, ["a", "b"], 1.5)));
test("no tiebreak single candidate", () =>
  assert.ok(!s.needsTiebreak({ a: 5.0 }, ["a"], 1.5)));
test("no tiebreak second zero", () =>
  assert.ok(!s.needsTiebreak({ a: 5.0, b: 0.0 }, ["a", "b"], 1.5)));

test("merge_refined reorders top-n keeps tail", () =>
  assert.deepStrictEqual(
    s.mergeRefined(["a", "b", "c", "d"], 2, { a: 1.0, b: 5.0 }), ["b", "a", "c", "d"]));
test("merge_refined all finalists", () =>
  assert.deepStrictEqual(s.mergeRefined(["a", "b"], 2, { a: 1.0, b: 2.0 }), ["b", "a"]));

// --- cache behavior (fake gh) ----------------------------------------------
function countingGh() {
  const gh = { graphqlCalls: 0, commitCalls: 0 };
  gh.graphql = async (query) => {
    gh.graphqlCalls++;
    const target = {};
    let i = 0;
    while (query.includes(`f${i}:`)) i++;
    for (let j = 0; j < Math.max(i, 1); j++) {
      target[`f${j}`] = { nodes: [{
        oid: `sha${j}`, additions: 10, deletions: 2,
        author: { user: { login: "alice" } },
        associatedPullRequests: { nodes: [] },
      }] };
    }
    return { repository: { defaultBranchRef: { target } } };
  };
  gh.commitFiles = async () => {
    gh.commitCalls++;
    return [{ filename: "a.cpp", additions: 5, deletions: 1 }];
  };
  return gh;
}

test("file history cached across calls", async () => {
  const gh = countingGh();
  const cache = new Map();
  await s.fetchFileHistories(gh, "o/r", ["a.cpp", "b.cpp"], "S", 5, cache);
  assert.strictEqual(gh.graphqlCalls, 1);
  await s.fetchFileHistories(gh, "o/r", ["a.cpp", "b.cpp"], "S", 5, cache);
  assert.strictEqual(gh.graphqlCalls, 1); // fully cached
  await s.fetchFileHistories(gh, "o/r", ["a.cpp", "c.cpp"], "S", 5, cache);
  assert.strictEqual(gh.graphqlCalls, 2); // only the uncached path
});

test("file history keyed by repo", async () => {
  const gh = countingGh();
  const cache = new Map();
  await s.fetchFileHistories(gh, "o/r1", ["a.cpp"], "S", 5, cache);
  await s.fetchFileHistories(gh, "o/r2", ["a.cpp"], "S", 5, cache);
  assert.strictEqual(gh.graphqlCalls, 2); // same path, different repo -> not shared
});

test("commit file loc cached by repo+sha", async () => {
  const gh = countingGh();
  const cache = new Map();
  assert.strictEqual(await s.commitFileLoc(gh, "o/r", "deadbeef", "a.cpp", cache), 5.0);
  await s.commitFileLoc(gh, "o/r", "deadbeef", "a.cpp", cache); // cached
  assert.strictEqual(gh.commitCalls, 1);
  await s.commitFileLoc(gh, "o/r", "cafef00d", "a.cpp", cache); // new sha
  assert.strictEqual(gh.commitCalls, 2);
});

// --- end-to-end ranking (fake gh) ------------------------------------------
test("compute_committer_ranking returns ranked logins", async () => {
  const gh = countingGh();
  const ranked = await s.computeCommitterRanking(gh, "o/r", "author", {
    locByFile: { "source/slang/a.cpp": 10.0 },
    fileMultipliers: TABLE,
    defaultMultiplier: 1.0,
    topFiles: 10,
    commits: 5,
    horizonDays: 180,
    botAuthors: BOTS,
    since: "S",
  });
  assert.deepStrictEqual(ranked, ["alice"]); // only alice authored the history
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
