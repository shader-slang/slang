// Protect one invocation with one -api option, enabled independent compile steps,
// the Windows API union and PR-only via-GLSL pass, separate Linux CPU/GPU masks,
// capability-specific roots/skip lists, and aggregate-gate failure handling.
// Execute the actual workflow commands without a GPU or downloaded build.
//
// This is an indentation-based extractor, not a general YAML parser: job keys
// have two spaces, named steps six, and run: | scripts ten. Conditions occupy
// one line and use the boolean expressions supported by stepRuns below. Shell
// rendering supports only inputs.* expressions; others fail before Bash runs.
// Run with: node .github/scripts/ci-test-consolidation.test.js
"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const { spawnSync } = require("node:child_process");
const { runInNewContext } = require("node:vm");
const root = path.resolve(__dirname, "../..");
const workflow = (name) =>
  fs.readFileSync(path.join(root, ".github/workflows", name), "utf8");
const ci = workflow("ci.yml");

// Return the raw caller block from ci.yml, excluding the following job.
function getJobText(id) {
  const start = ci.indexOf(`  ${id}:\n`);
  assert.notEqual(start, -1, `missing job ${id}`);
  const rest = ci.slice(start + 1);
  const end = rest.search(/\n  [a-z][\w-]*:\n/);
  return end < 0 ? rest : rest.slice(0, end);
}

// Return the named step's raw metadata and body from a reusable workflow.
function getStepText(text, name) {
  const start = text.indexOf(`      - name: ${name}\n`);
  assert.notEqual(start, -1, `missing step ${name}`);
  const rest = text.slice(start);
  const end = rest.indexOf("\n      - ");
  return end < 0 ? rest : rest.slice(0, end);
}

// Extract and dedent a literal run: | body; folded run: > blocks are unsupported.
function getStepScript(text, name) {
  const source = getStepText(text, name);
  const run = source.indexOf("        run: |\n");
  assert.notEqual(run, -1, `missing literal script for ${name}`);
  return source
    .slice(run + "        run: |\n".length)
    .split("\n")
    .map((line) => (line.startsWith("          ") ? line.slice(10) : line))
    .join("\n");
}

// These explicit conditions use boolean operators shared with JavaScript.
// Evaluate the actual expression with successful preceding steps; bracket input
// lookups accommodate Actions input names containing hyphens.
function stepRuns(text, name, inputs, event) {
  const condition = getStepText(text, name).match(/^        if: (.+)$/m);
  assert.ok(condition, `missing condition for ${name}`);
  const expression = condition[1].replace(
    /inputs\.([\w-]+)/g,
    (_, key) => `inputs[${JSON.stringify(key)}]`,
  );
  return runInNewContext(
    expression,
    {
      inputs,
      github: { event_name: event },
      always: () => true,
    },
    { timeout: 1000 },
  );
}

// Substitute supplied inputs and reject expressions this harness cannot render.
function renderWorkflowInputs(source, inputs) {
  const rendered = source.replace(
    /\$\{\{\s*inputs\.([\w-]+)\s*\}\}/g,
    (_, key) => {
      assert.ok(key in inputs, `unprovided workflow input ${key}`);
      return String(inputs[key]);
    },
  );
  assert.ok(
    !rendered.includes("${{"),
    "Unsupported workflow expression in step script",
  );
  return rendered;
}

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "ci-consolidation-"));
try {
  const capture = path.join(tmp, "args.json");
  fs.writeFileSync(
    path.join(tmp, "slang-test"),
    '#!/usr/bin/env node\nrequire("fs").appendFileSync(process.env.CAPTURE, JSON.stringify(process.argv.slice(2)) + "\\n");\n',
    { mode: 0o755 },
  );
  // Check Bash syntax, execute the rendered body, and return its process result.
  function runStepScript(source, inputs, extraEnv = {}) {
    const shell = renderWorkflowInputs(source, inputs);
    const syntax = spawnSync("/bin/bash", ["-n"], {
      input: shell,
      encoding: "utf8",
    });
    assert.equal(syntax.status, 0, syntax.stderr);
    const result = spawnSync("/bin/bash", ["-e", "-c", shell], {
      cwd: root,
      encoding: "utf8",
      env: { ...process.env, bin_dir: tmp, CAPTURE: capture, ...extraEnv },
    });
    return result;
  }
  // Execute a step and return its single captured argv after checking API policy.
  function runAndCaptureSlangTestArgs(source, inputs, extraEnv) {
    fs.writeFileSync(capture, "");
    const result = runStepScript(source, inputs, extraEnv);
    assert.equal(result.status, 0, result.stdout + result.stderr);
    const records = fs
      .readFileSync(capture, "utf8")
      .trim()
      .split("\n")
      .filter(Boolean);
    assert.equal(
      records.length,
      1,
      "Expected exactly one slang-test invocation",
    );
    const args = JSON.parse(records[0]);
    assert.equal(args.filter((arg) => arg === "-api").length, 1);
    assert.ok(!args.includes("-api-only"));
    assert.ok(!args.includes("-skip-api-independent-tests"));
    return args;
  }
  const ordinary = workflow("ci-slang-test.yml");
  const container = workflow("ci-slang-test-container.yml");
  for (const config of ["debug", "release"]) {
    const id = `test-windows-${config}-cl-x86_64-gpu`;
    const caller = getJobText(id);
    const apiInput = caller.match(/^      api: (\S+)$/m)?.[1];
    assert.ok(apiInput, `missing API union in ${id}`);
    const selectedApis = new Set(apiInput.split("+"));
    for (const api of ["dx11", "dx12", "wgpu", "vk", "cuda", "cpu", "llvm"]) {
      assert.ok(selectedApis.has(api), `${id} must retain ${api} coverage`);
    }
    assert.match(caller, /^      run-via-glsl: true$/m);
    const inputs = {
      os: "windows",
      config,
      "cpu-only": false,
      "gpu-api-only": false,
      "enable-debug-layers": true,
      "server-count": 8,
      "gpu-tier": "t4",
      "full-gpu-tests": true,
      "run-via-glsl": true,
    };
    for (const name of [
      "Test Slang via glsl",
      "Check via-glsl slang-test worktree cleanliness",
    ]) {
      for (const [event, expected] of [
        ["pull_request", true],
        ["merge_group", false],
        ["push", false],
      ]) {
        assert.equal(
          stepRuns(ordinary, name, inputs, event),
          expected,
          `${config}/${event}: ${name} condition`,
        );
      }
      for (const disabled of [
        { "run-via-glsl": false },
        { "full-gpu-tests": false },
        { os: "macos" },
      ]) {
        assert.equal(
          stepRuns(ordinary, name, { ...inputs, ...disabled }, "pull_request"),
          false,
          `${config}: ${name} disabled by ${JSON.stringify(disabled)}`,
        );
      }
    }
    const args = runAndCaptureSlangTestArgs(
      getStepScript(ordinary, "Test Slang"),
      inputs,
      {
        API_INPUT: apiInput,
      },
    );
    assert.deepEqual(
      new Set(args[args.indexOf("-api") + 1].split("+")),
      selectedApis,
    );
    assert.ok(args.includes("tests/skip-list-gpu-t4.txt"));
    assert.equal(
      args.includes("tests/skip-list-debug.txt"),
      config === "debug",
    );
    const via = runAndCaptureSlangTestArgs(
      getStepScript(ordinary, "Test Slang via glsl"),
      inputs,
    );
    assert.equal(via[via.indexOf("-api") + 1], "vk");
    assert.ok(via.includes("-emit-spirv-via-glsl"));
    assert.match(getJobText("check-ci"), new RegExp(`        ${id},`));
    console.log(
      `PASS Windows ${config}: one combined main and one via-GLSL pass`,
    );
  }
  const cpuCaller = getJobText("test-linux-release-gcc-x86_64-cpu");
  assert.match(cpuCaller, /^      full-gpu-tests: false$/m);
  assert.match(cpuCaller, /^      cpu-only: true$/m);
  assert.match(cpuCaller, /^      slang-user-skills: true$/m);
  const cpuArgs = runAndCaptureSlangTestArgs(
    getStepScript(ordinary, "Test Slang"),
    {
      os: "linux",
      config: "release",
      "cpu-only": true,
      "gpu-api-only": false,
      "enable-debug-layers": true,
      "server-count": 4,
      "gpu-tier": "",
      "full-gpu-tests": false,
    },
    { API_INPUT: "" },
  );
  assert.equal(cpuArgs[cpuArgs.indexOf("-api") + 1], "cpu+llvm");
  console.log(
    "PASS Linux CPU/LLVM: separate non-GPU hosted job and skills checks",
  );
  for (const config of ["debug", "release"]) {
    for (const tier of ["t4", "sm80plus"]) {
      const args = runAndCaptureSlangTestArgs(
        getStepScript(container, "Test Slang"),
        {
          config,
          "server-count": 8,
          "gpu-tier": tier,
        },
      );
      const apis = args[args.indexOf("-api") + 1].split("+");
      for (const api of ["vk", "cuda"]) assert.ok(apis.includes(api));
      for (const api of ["cpu", "llvm"]) assert.ok(!apis.includes(api));
      assert.equal(args.includes("tests/skip-list-gpu-t4.txt"), tier === "t4");
      assert.equal(args.includes("tests/neural/"), tier === "sm80plus");
      assert.equal(
        args.includes("tests/cooperative-matrix/"),
        tier === "sm80plus",
      );
      console.log(
        `PASS Linux ${config} ${tier}: GPU-only mask and capability selection`,
      );
    }
  }
  assert.match(
    getJobText("test-linux-release-gcc-x86_64-sm80"),
    /^      gpu-tier: sm80plus$/m,
  );
  for (const id of [
    "test-linux-debug-gcc-x86_64",
    "test-linux-release-gcc-x86_64",
    "test-linux-release-gcc-x86_64-sm80",
    "test-linux-release-gcc-x86_64-cpu",
  ])
    assert.match(getJobText("check-ci"), new RegExp(`        ${id},`));

  const gate = getStepScript(ci, "Check CI Results");
  const required = [
    "test-windows-debug-cl-x86_64-gpu",
    "test-windows-release-cl-x86_64-gpu",
    "test-linux-debug-gcc-x86_64",
    "test-linux-release-gcc-x86_64",
    "test-linux-release-gcc-x86_64-sm80",
    "test-linux-release-gcc-x86_64-cpu",
  ];
  for (const failing of [null, ...required]) {
    const needs = Object.fromEntries(
      required.map((name) => [
        name,
        { result: name === failing ? "failure" : "success" },
      ]),
    );
    const result = runStepScript(
      gate,
      {},
      {
        NEEDS_JSON: JSON.stringify(needs),
        FILTER_RESULT: "success",
        SHOULD_RUN: "true",
      },
    );
    assert.equal(result.status, failing ? 1 : 0, result.stdout + result.stderr);
  }
  const skipped = Object.fromEntries(
    required.map((name) => [name, { result: "skipped" }]),
  );
  assert.equal(
    runStepScript(
      gate,
      {},
      {
        NEEDS_JSON: JSON.stringify(skipped),
        FILTER_RESULT: "success",
        SHOULD_RUN: "false",
      },
    ).status,
    0,
  );
  assert.equal(
    runStepScript(
      gate,
      {},
      {
        NEEDS_JSON: JSON.stringify(skipped),
        FILTER_RESULT: "success",
        SHOULD_RUN: "true",
      },
    ).status,
    1,
  );
  console.log(
    "PASS aggregate gate: success, each required failure, and expected/unexpected skips",
  );
} finally {
  fs.rmSync(tmp, { recursive: true, force: true });
}
