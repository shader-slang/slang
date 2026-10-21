// Exercise the actual workflow commands without a GPU or downloaded build.
// Run with: node .github/scripts/ci-test-consolidation.test.js
"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const { spawnSync } = require("node:child_process");
const root = path.resolve(__dirname, "../..");
const workflow = (name) =>
  fs.readFileSync(path.join(root, ".github/workflows", name), "utf8");
const ci = workflow("ci.yml");

function job(id) {
  const start = ci.indexOf(`  ${id}:\n`);
  assert.notEqual(start, -1, `missing job ${id}`);
  const rest = ci.slice(start + 1);
  const end = rest.search(/\n  [a-z][\w-]*:\n/);
  return end < 0 ? rest : rest.slice(0, end);
}

function script(text, name) {
  const start = text.indexOf(`      - name: ${name}\n`);
  assert.notEqual(start, -1, `missing step ${name}`);
  const rest = text.slice(start);
  const end = rest.indexOf("\n      - ");
  const step = end < 0 ? rest : rest.slice(0, end);
  const run = step.indexOf("        run: |\n");
  assert.notEqual(run, -1, `missing literal script for ${name}`);
  return step
    .slice(run + "        run: |\n".length)
    .split("\n")
    .map((line) => (line.startsWith("          ") ? line.slice(10) : line))
    .join("\n");
}

function render(source, inputs) {
  return source.replace(/\$\{\{ inputs\.([\w-]+) \}\}/g, (_, key) => {
    assert.ok(key in inputs, `unprovided workflow input ${key}`);
    return String(inputs[key]);
  });
}

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "ci-consolidation-"));
try {
  const capture = path.join(tmp, "args.json");
  fs.writeFileSync(
    path.join(tmp, "slang-test"),
    '#!/usr/bin/env node\nrequire("fs").writeFileSync(process.env.CAPTURE, JSON.stringify(process.argv.slice(2)));\n',
    { mode: 0o755 },
  );
  function run(source, inputs, extraEnv = {}) {
    const shell = render(source, inputs);
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
  function command(source, inputs, extraEnv) {
    const result = run(source, inputs, extraEnv);
    assert.equal(result.status, 0, result.stdout + result.stderr);
    const args = JSON.parse(fs.readFileSync(capture, "utf8"));
    assert.equal(args.filter((arg) => arg === "-api").length, 1);
    assert.ok(!args.includes("-api-only"));
    assert.ok(!args.includes("-skip-api-independent-tests"));
    return args;
  }
  const ordinary = workflow("ci-slang-test.yml");
  const container = workflow("ci-slang-test-container.yml");
  for (const config of ["debug", "release"]) {
    const id = `test-windows-${config}-cl-x86_64-gpu`;
    const caller = job(id);
    assert.match(caller, /api: dx11\+dx12\+wgpu\+vk\+cuda\+cpu\+llvm/);
    assert.match(caller, /run-via-glsl: true/);
    const inputs = {
      os: "windows",
      config,
      "cpu-only": false,
      "gpu-api-only": false,
      "enable-debug-layers": true,
      "server-count": 8,
      "gpu-tier": "t4",
      "full-gpu-tests": true,
    };
    const args = command(script(ordinary, "Test Slang"), inputs, {
      API_INPUT: "dx11+dx12+wgpu+vk+cuda+cpu+llvm",
    });
    assert.equal(
      args[args.indexOf("-api") + 1],
      "dx11+dx12+wgpu+vk+cuda+cpu+llvm",
    );
    assert.ok(args.includes("tests/skip-list-gpu-t4.txt"));
    assert.equal(
      args.includes("tests/skip-list-debug.txt"),
      config === "debug",
    );
    const via = command(script(ordinary, "Test Slang via glsl"), inputs);
    assert.equal(via[via.indexOf("-api") + 1], "vk");
    assert.ok(via.includes("-emit-spirv-via-glsl"));
    assert.match(job("check-ci"), new RegExp(`        ${id},`));
    for (const api of ["dx", "vk", "cuda"])
      assert.ok(!ci.includes(`${id}-${api}:`));
    console.log(
      `PASS Windows ${config}: one combined main and one via-GLSL pass`,
    );
  }
  assert.match(job("test-windows-debug-cl-x86_64-gpu"), /timeout-minutes: 120/);
  assert.ok(!ci.includes("test-linux-release-gcc-x86_64-cpu"));
  assert.match(job("test-linux-release-gcc-x86_64"), /slang-user-skills: true/);
  assert.match(container, /export SLANG_DISABLE_AVX512=1/);
  assert.match(container, /Validate Slang user-skill examples/);
  for (const config of ["debug", "release"]) {
    for (const tier of ["t4", "sm80plus"]) {
      const args = command(script(container, "Test Slang"), {
        config,
        "server-count": 8,
        "gpu-tier": tier,
      });
      const apis = args[args.indexOf("-api") + 1].split("+");
      for (const api of ["vk", "cuda", "cpu", "llvm"])
        assert.ok(apis.includes(api));
      assert.equal(args.includes("tests/skip-list-gpu-t4.txt"), tier === "t4");
      assert.equal(args.includes("tests/neural/"), tier === "sm80plus");
      assert.equal(
        args.includes("tests/cooperative-matrix/"),
        tier === "sm80plus",
      );
      console.log(
        `PASS Linux ${config} ${tier}: CPU/LLVM coverage and capability selection`,
      );
    }
  }
  assert.match(job("test-linux-release-gcc-x86_64-sm80"), /"SM80Plus"/);
  for (const id of [
    "test-linux-debug-gcc-x86_64",
    "test-linux-release-gcc-x86_64",
    "test-linux-release-gcc-x86_64-sm80",
  ])
    assert.match(job("check-ci"), new RegExp(`        ${id},`));

  const gate = script(ci, "Check CI Results");
  const required = [
    "test-windows-debug-cl-x86_64-gpu",
    "test-windows-release-cl-x86_64-gpu",
    "test-linux-debug-gcc-x86_64",
    "test-linux-release-gcc-x86_64",
    "test-linux-release-gcc-x86_64-sm80",
  ];
  for (const failing of [null, ...required]) {
    const needs = Object.fromEntries(
      required.map((name) => [
        name,
        { result: name === failing ? "failure" : "success" },
      ]),
    );
    const result = run(
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
    run(
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
    run(
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
