// Unit tests for assignee/reviewer selection, run against the copy INLINED in
// pr-board-sync.yml (the single source of truth), extracted at run time.
// No deps; run with: node .github/scripts/pr-assign.test.js
"use strict";

const assert = require("node:assert");
const {
  selectAssigneeAndReviewers,
  pickSuggestedReviewer,
  formatAssignmentComment,
} = require("./extract-workflow-js.js").load({
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
  const { assignee, reviewers, suggestedReviewer } = select([], ["dev1"]);
  assert.strictEqual(assignee, "maintainer");
  assert.deepStrictEqual(reviewers, ["maintainer"]);
  assert.strictEqual(suggestedReviewer, "dev1");
});

test("auto-request is assignee only; non-owner is suggested not requested", () => {
  const { assignee, reviewers, suggestedReviewer } = select(
    [], ["dev2", "owner1", "dev1"]);
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, ["owner1"]);
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("no suggestion when assignee has top signal", () => {
  const { reviewers, suggestedReviewer } = select([], ["owner1", "owner2", "dev1"]);
  assert.deepStrictEqual(reviewers, ["owner1"]);
  assert.strictEqual(suggestedReviewer, null);
});

test("no collaborator committer means only assignee", () => {
  const { reviewers, suggestedReviewer } = select([], ["owner1", "owner2"]);
  assert.deepStrictEqual(reviewers, ["owner1"]);
  assert.strictEqual(suggestedReviewer, null);
});

test("author never requested as reviewer", () => {
  const { assignee, reviewers, suggestedReviewer } = selectAssigneeAndReviewers({
    issueAssignees: [], committersBySignal: ["dev1"], owners: OWNERS,
    collaborators: COLLAB, author: "maintainer", maintainer: "maintainer",
  });
  assert.strictEqual(assignee, "maintainer");
  assert.deepStrictEqual(reviewers, []); // author(maintainer) excluded from request
  assert.strictEqual(suggestedReviewer, "dev1");
});

test("real existing reviewer blocks adding but still suggests", () => {
  const { assignee, reviewers, suggestedReviewer } = select([], ["dev2", "owner1"], {
    existingReviewers: ["dave"], ignoredReviewers: new Set(["bmillsNV"]),
  });
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, []);
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("ignored and bot reviewers do not count as existing", () => {
  const { assignee, reviewers, suggestedReviewer } = select([], ["dev2", "owner1"], {
    existingReviewers: ["bmillsNV", "copilot[bot]"],
    botAuthors: ["nv-slang-bot"], ignoredReviewers: new Set(["bmillsNV"]),
  });
  assert.strictEqual(assignee, "owner1");
  assert.deepStrictEqual(reviewers, ["owner1"]);
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("pickSuggestedReviewer skips non-collaborators and bots", () => {
  assert.strictEqual(
    pickSuggestedReviewer({
      committersBySignal: ["outsider", "bot[bot]", "dev1", "owner1"],
      collaborators: COLLAB,
      author: "author",
      assignee: "owner1",
      botAuthors: [],
    }),
    "dev1",
  );
});

test("formatAssignmentComment always notes assignee; suggestion has no @", () => {
  assert.strictEqual(
    formatAssignmentComment({
      source: "Bot", assignee: "jkwak-work", suggestedReviewer: null,
    }),
    "**PR board sync:** auto-assigned @jkwak-work as shepherd for this Bot PR.",
  );
  const withSuggestion = formatAssignmentComment({
    source: "Community",
    assignee: "alice",
    suggestedReviewer: "skallweitNV",
  });
  assert.match(withSuggestion, /^\*\*PR board sync:\*\* auto-assigned @alice as shepherd for this Community PR\./);
  assert.match(withSuggestion, /higher for skallweitNV than/);
  assert.doesNotMatch(withSuggestion, /@skallweitNV/);
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
