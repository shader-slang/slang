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
  const { assignee, autoRequestedReviewer } = select(["owner2"], ["owner1", "dev1"]);
  assert.strictEqual(assignee, "owner2");
  assert.strictEqual(autoRequestedReviewer, "owner2");
});

test("commit-signal owner when no issue", () => {
  const { assignee } = select([], ["owner1", "owner2"]);
  assert.strictEqual(assignee, "owner1");
});

test("maintainer fallback", () => {
  const { assignee, autoRequestedReviewer, autoRequestedHasSignal, suggestedReviewer } =
    select([], ["dev1"]);
  assert.strictEqual(assignee, "maintainer");
  assert.strictEqual(autoRequestedReviewer, "maintainer");
  assert.strictEqual(autoRequestedHasSignal, false); // absent from ranking
  assert.strictEqual(suggestedReviewer, "dev1");
});

test("auto-request is shepherd only; higher-signal collaborator is suggested", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } = select(
    [], ["dev2", "owner1", "dev1"]);
  assert.strictEqual(assignee, "owner1");
  assert.strictEqual(autoRequestedReviewer, "owner1");
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("may suggest another owner who outranks the auto-requested shepherd", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } = select(
    ["owner1"], ["owner2", "owner1", "dev1"]);
  assert.strictEqual(assignee, "owner1");
  assert.strictEqual(autoRequestedReviewer, "owner1");
  assert.strictEqual(suggestedReviewer, "owner2");
});

test("no suggestion when auto-requested reviewer has top signal", () => {
  const { autoRequestedReviewer, suggestedReviewer } =
    select([], ["owner1", "owner2", "dev1"]);
  assert.strictEqual(autoRequestedReviewer, "owner1");
  assert.strictEqual(suggestedReviewer, null);
});

test("no collaborator above reviewer means only assignee", () => {
  const { autoRequestedReviewer, suggestedReviewer } =
    select([], ["owner1", "owner2"]);
  assert.strictEqual(autoRequestedReviewer, "owner1");
  assert.strictEqual(suggestedReviewer, null);
});

test("author shepherd is not auto-requested; top collaborator is suggested", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } =
    selectAssigneeAndReviewers({
      issueAssignees: [],
      committersBySignal: ["dev1", "owner1"],
      owners: OWNERS,
      collaborators: COLLAB,
      author: "owner1",
      maintainer: MAINT,
    });
  assert.strictEqual(assignee, "owner1");
  assert.strictEqual(autoRequestedReviewer, null);
  assert.strictEqual(suggestedReviewer, "dev1");
});

test("ignored shepherd is not auto-requested; suggestion uses null baseline", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } = select(
    [], ["dev2"], {
      maintainer: "bmillsNV",
      ignoredReviewers: new Set(["bmillsNV"]),
    });
  // No owners in ranking → maintainer fallback, but ignored → no auto-request.
  assert.strictEqual(assignee, "bmillsNV");
  assert.strictEqual(autoRequestedReviewer, null);
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("real existing reviewer clears auto-request but still may suggest", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } = select(
    [], ["dev2", "owner1"], {
      existingReviewers: ["dave"], ignoredReviewers: new Set(["bmillsNV"]),
    });
  assert.strictEqual(assignee, "owner1");
  assert.strictEqual(autoRequestedReviewer, null);
  // Baseline is null, but assignee is excluded from suggestion → next is dev2.
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("ignored and bot reviewers do not count as existing", () => {
  const { assignee, autoRequestedReviewer, suggestedReviewer } = select(
    [], ["dev2", "owner1"], {
      existingReviewers: ["bmillsNV", "copilot[bot]"],
      botAuthors: ["nv-slang-bot"], ignoredReviewers: new Set(["bmillsNV"]),
    });
  assert.strictEqual(assignee, "owner1");
  assert.strictEqual(autoRequestedReviewer, "owner1");
  assert.strictEqual(suggestedReviewer, "dev2");
});

test("pickSuggestedReviewer skips non-collaborators, bots, and assignee", () => {
  assert.strictEqual(
    pickSuggestedReviewer({
      committersBySignal: ["outsider", "bot[bot]", "dev1", "owner1"],
      collaborators: COLLAB,
      author: "author",
      assignee: "owner1",
      autoRequestedReviewer: "owner1",
      botAuthors: [],
    }),
    "dev1",
  );
});

test("pickSuggestedReviewer with no auto-request takes top non-assignee collaborator", () => {
  assert.strictEqual(
    pickSuggestedReviewer({
      committersBySignal: ["owner1", "dev2"],
      collaborators: COLLAB,
      author: "author",
      assignee: "owner1",
      autoRequestedReviewer: null,
      botAuthors: [],
    }),
    "dev2",
  );
});

test("pickSuggestedReviewer skips a named bot from botAuthors", () => {
  assert.strictEqual(
    pickSuggestedReviewer({
      committersBySignal: ["nv-slang-bot", "dev1", "owner1"],
      collaborators: new Set(["nv-slang-bot", "dev1", "owner1"]),
      author: "author",
      assignee: "owner1",
      autoRequestedReviewer: "owner1",
      botAuthors: ["nv-slang-bot"],
    }),
    "dev1",
  );
});

test("formatAssignmentComment always notes assignee; suggestion has no @", () => {
  assert.strictEqual(
    formatAssignmentComment({
      source: "Bot",
      assignee: "jkwak-work",
      suggestedReviewer: null,
      autoRequestedReviewer: "jkwak-work",
      autoRequestedHasSignal: true,
    }),
    "**PR board sync:** auto-assigned @jkwak-work as shepherd for this Bot PR.",
  );
  const withRequested = formatAssignmentComment({
    source: "Community",
    assignee: "alice",
    suggestedReviewer: "skallweitNV",
    autoRequestedReviewer: "alice",
    autoRequestedHasSignal: true,
  });
  assert.match(
    withRequested,
    /^\*\*PR board sync:\*\* auto-assigned @alice as shepherd for this Community PR\./,
  );
  assert.match(
    withRequested,
    /higher for skallweitNV than for the auto-requested reviewer \(alice\)/,
  );
  assert.doesNotMatch(withRequested, /@skallweitNV/);

  const withoutRequested = formatAssignmentComment({
    source: "Bot",
    assignee: "author-owner",
    suggestedReviewer: "dev1",
    autoRequestedReviewer: null,
    autoRequestedHasSignal: false,
  });
  assert.match(withoutRequested, /highest for dev1 among collaborators/);
  assert.doesNotMatch(withoutRequested, /@dev1/);

  // Auto-requested maintainer with no measured signal: do not claim they have
  // "lower" signal than the suggestion.
  const noSignalBaseline = formatAssignmentComment({
    source: "Community",
    assignee: "maintainer",
    suggestedReviewer: "dev1",
    autoRequestedReviewer: "maintainer",
    autoRequestedHasSignal: false,
  });
  assert.match(noSignalBaseline, /highest for dev1 among collaborators/);
  assert.doesNotMatch(noSignalBaseline, /than for the auto-requested reviewer/);
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
