// Unit tests for the GITHUB_TOKEN write adapter inlined in pr-board-sync.yml.
// No deps; run with: node .github/scripts/pr-write-identity.test.js
"use strict";

const assert = require("node:assert");
const {
  actionsWrite,
  repoApiPath,
} = require("./extract-workflow-js.js").load({
  workflow: ".github/workflows/pr-board-sync.yml",
  block: "actions-write",
});

const tests = [];
const test = (name, fn) => tests.push([name, fn]);

function setGlobals(fetchImpl) {
  const warnings = [];
  global.fetch = fetchImpl;
  global.core = { warning: (message) => warnings.push(message) };
  global.context = { repo: { owner: "shader-slang", repo: "slang" } };
  process.env.ACTIONS_TOKEN = "actions-token";
  process.env.GITHUB_API_URL = "https://api.github.test";
  return warnings;
}

test("repoApiPath targets the caller repository", () => {
  setGlobals(async () => ({ ok: true, status: 200 }));
  assert.strictEqual(
    repoApiPath("/issues/42/assignees"),
    "/repos/shader-slang/slang/issues/42/assignees",
  );
});

test("successful writes use GITHUB_TOKEN and do not call the PAT fallback", async () => {
  let request;
  let fallbackCalls = 0;
  setGlobals(async (url, options) => {
    request = { url, options };
    return { ok: true, status: 201 };
  });

  await actionsWrite(
    "POST",
    "/repos/shader-slang/slang/issues/42/assignees",
    { assignees: ["owner"] },
    async () => { fallbackCalls++; },
  );

  assert.strictEqual(fallbackCalls, 0);
  assert.strictEqual(
    request.url,
    "https://api.github.test/repos/shader-slang/slang/issues/42/assignees",
  );
  assert.strictEqual(request.options.headers.authorization, "Bearer actions-token");
  assert.deepStrictEqual(
    JSON.parse(request.options.body),
    { assignees: ["owner"] },
  );
});

test("a permission denial uses the PAT fallback when one is allowed", async () => {
  let fallbackCalls = 0;
  const warnings = setGlobals(async () => ({
    ok: false,
    status: 403,
    text: async () => "Resource not accessible by integration",
  }));

  const result = await actionsWrite(
    "DELETE",
    "/repos/shader-slang/slang/pulls/42/requested_reviewers",
    { reviewers: ["ignored"] },
    async () => {
      fallbackCalls++;
      return "fallback-result";
    },
  );

  assert.strictEqual(result, "fallback-result");
  assert.strictEqual(fallbackCalls, 1);
  assert.match(warnings[0], /using the PAT fallback/);
});

test("comments cannot fall back to the PAT on a permission denial", async () => {
  setGlobals(async () => ({
    ok: false,
    status: 403,
    text: async () => "Resource not accessible by integration",
  }));

  await assert.rejects(
    actionsWrite(
      "POST",
      "/repos/shader-slang/slang/issues/42/comments",
      { body: "assignment notice" },
    ),
    /403 Resource not accessible by integration/,
  );
});

test("non-permission failures never fall back to the PAT", async () => {
  let fallbackCalls = 0;
  setGlobals(async () => ({
    ok: false,
    status: 500,
    text: async () => "server error",
  }));

  await assert.rejects(
    actionsWrite(
      "POST",
      "/repos/shader-slang/slang/issues/42/assignees",
      { assignees: ["owner"] },
      async () => { fallbackCalls++; },
    ),
    /500 server error/,
  );
  assert.strictEqual(fallbackCalls, 0);
});

(async () => {
  let passed = 0, failed = 0;
  for (const [name, fn] of tests) {
    try {
      await fn();
      passed++;
    } catch (error) {
      failed++;
      console.error(`FAIL: ${name}\n  ${(error && error.stack) || error}`);
    }
  }
  console.log(`${passed} passed, ${failed} failed`);
  process.exit(failed ? 1 : 0);
})();
