// Unit tests for the assignee/reviewer selection (the JS port of
// select_assignee_and_reviewers), run against the copy INLINED in
// pr-board-sync.yml (the single source of truth), extracted at run time.
// No deps; run with: node .github/scripts/pr-assign.test.js
// Mirrors TestSelectAssigneeAndReviewers in the slang-skills test_pr_sweep.py.
"use strict";

const assert = require("node:assert");
const { selectAssigneeAndReviewers } = require("./extract-workflow-js.js").load({
  workflow: ".github/workflows/pr-board-sync.yml",
  block: "assignment",
});

const OWNERS = new Set(["owner1", "owner2"]);
const COLLAB = new Set(["owner1", "owner2", "dev1", "dev2"]); // owners are also collaborators
const MAINT = "maintainer";

function select(issue, committers, extra) {
  return selectAssigneeAndReviewers({
    issueAssignees: issue, committersBySignal: committers,
    owners: OWNERS, collaborators: COLLAB, author: "author", maintainer: MAINT,
    ...(extra || {}),
  });
}

const tests = [];
const test = (name, fn) => tests.push([name, fn]);

test("issue assignee wins", () => {
  const { assignee, reviewers } = select(["owner2"], ["owner1", "dev1"]);
  assert.strictEqual(assignee, "owner2");
  assert.ok(reviewers.includes("owner2"));
});

test("commit-signal owner when no issue", () => {
  const { assignee } = select([], ["owner1", "owner2"]);
  assert.strictEqual(assignee, "owner1");
});

test("maintainer fallback", () => {
  const { assignee, reviewers } = select([], ["dev1"]);
  assert.strictEqual(assignee, "maintainer");
  assert.deepStrictEqual(reviewers, ["maintainer", "dev1"]);
});

test("extra reviewer is top collaborator-not-owner", () => {
  const { assignee, reviewers } = select([], ["dev2", "owner1", "dev1"]);
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, ["owner1", "dev2"]);
});

test("no collaborator committer means only assignee", () => {
  const { reviewers } = select([], ["owner1", "owner2"]);
  assert.deepStrictEqual(reviewers, ["owner1"]);
});

test("author never requested as reviewer", () => {
  const { assignee, reviewers } = selectAssigneeAndReviewers({
    issueAssignees: [], committersBySignal: ["dev1"], owners: OWNERS,
    collaborators: COLLAB, author: "maintainer", maintainer: "maintainer",
  });
  assert.strictEqual(assignee, "maintainer");
  assert.deepStrictEqual(reviewers, ["dev1"]); // author(maintainer) excluded
});

test("real existing reviewer blocks adding", () => {
  const { assignee, reviewers } = select([], ["dev2", "owner1"], {
    existingReviewers: ["dave"], ignoredReviewers: new Set(["bmillsNV"]),
  });
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, []);
});

test("ignored and bot reviewers do not count as existing", () => {
  const { assignee, reviewers } = select([], ["dev2", "owner1"], {
    existingReviewers: ["bmillsNV", "copilot[bot]"],
    botAuthors: ["nv-slang-bot"], ignoredReviewers: new Set(["bmillsNV"]),
  });
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, ["owner1", "dev2"]);
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
