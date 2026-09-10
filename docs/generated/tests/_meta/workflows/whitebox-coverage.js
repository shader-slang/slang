export const meta = {
  name: "whitebox-coverage-sweep",
  description:
    "Broad-sweep white-box characterization tests targeting uncovered CLI-reachable compiler code, per coverage/METHODOLOGY.md",
  phases: [
    {
      title: "Whitebox",
      detail: "one agent per reachable source-area cluster",
    },
  ],
};

// Reachable clusters from the row-11 gap triage. Each: coverage/ bundle name,
// the target source file(s) + current line-cov%, and a reachable-input hint.
// EXCLUDED (unreachable in this build / not .slang-testable): slang-emit-llvm
// (LLVM backend DISABLED), reflection-api/json + json-value (C++ API),
// ast-print (-dump-ast, unmaintained).
const CLUSTERS = [
  {
    bundle: "check-decl",
    files: "source/slang/slang-check-decl.cpp (63%)",
    hint: "diverse declaration forms, modifiers, visibility (public/internal/private), initializers, generic/extension/typedef/enum/property/subscript decls, and especially documented error/diagnostic paths",
  },
  {
    bundle: "check-expr",
    files: "source/slang/slang-check-expr.cpp (56%)",
    hint: "expression checking: overload/operator resolution, implicit conversions, swizzles, initializer lists, casts, and error paths (type-mismatch, ambiguous-call diagnostics)",
  },
  {
    bundle: "autodiff",
    files:
      "source/slang/slang-ir-autodiff-fwd.cpp (35%), slang-ir-autodiff-transpose.cpp (25%), slang-ir-autodiff-primal-hoist.cpp (44%)",
    hint: "fwd_diff()/bwd_diff() over differentiable functions with control flow (if/for/while), nested calls, [Differentiable] structs, DifferentialPair. WARNING: derivative correctness is hard to eyeball — tag tests characterization-unverified=true and pin only the structural emitted tokens you can confirm; a wrong derivative is a finding, not a green test",
  },
  {
    bundle: "specialize",
    files:
      "source/slang/slang-ir-typeflow-specialize.cpp (41%), slang-ir-specialize.cpp (34%)",
    hint: "generic specialization, interface/existential conformance, dynamic dispatch, associated types, generic value parameters",
  },
  {
    bundle: "lower-to-ir",
    files: "source/slang/slang-lower-to-ir.cpp (66%)",
    hint: "lowering of varied constructs: defer, switch, labeled break/continue, compound assignment, ternary, struct/array init, global/static var, multi-level control flow",
  },
  {
    bundle: "parser",
    files: "source/slang/slang-parser.cpp (68%)",
    hint: "syntax edge cases: attribute syntax, generic-arg syntax, operator declarations, layout qualifiers, expression-statement forms, and parse-error recovery (diagnostics)",
  },
  {
    bundle: "emit",
    files:
      "source/slang/slang-emit-spirv.cpp (60%), slang-emit-glsl.cpp (50%), slang-emit-c-like.cpp (62%)",
    hint: "deeper emit paths beyond the breadth/depth passes: less-common intrinsics, control-flow shapes (switch, multi-level break), struct/array return, matrix ops, that emit to spirv-asm/glsl/cpp",
  },
  {
    bundle: "legalize",
    files:
      "source/slang/slang-ir-glsl-legalize.cpp (63%), slang-ir-legalize-types.cpp (48%), slang-ir-legalize-varying-params.cpp (61%)",
    hint: "type/varying-param legalization: varying in/out struct flattening, system-value semantics, resource-in-struct legalization, entry-point parameter shapes across spirv/glsl",
  },
  {
    bundle: "type-layout",
    files: "source/slang/slang-type-layout.cpp (57%)",
    hint: "layout rules: cbuffer std140 vs structured-buffer std430, nested struct/array layout, matrix layout, ParameterBlock, push-constant, observable via emitted offsets/strides on spirv-asm/glsl/hlsl",
  },
  {
    bundle: "cli-options",
    files: "source/slang/slang-options.cpp (57%)",
    hint: "slangc command-line option handling: flag combinations and their observable effect (-O0..-O3, -g, -fvk-* flags, -profile, -capability, -default-image-format-unknown, -matrix-layout-*, -line-directive-mode). Use multiple //TEST:SIMPLE directives with different flags; pin the observable difference in emitted output",
  },
  {
    bundle: "torch",
    files: "source/slang/slang-ir-pytorch-cpp-binding.cpp (15%)",
    hint: "PyTorch binding emission via -target torch / TorchTensor / [CudaKernel]/[TorchEntryPoint]; may need //META: requires-tool — pin the emitted binding shape",
  },
];

const RESULT_SCHEMA = {
  type: "object",
  additionalProperties: false,
  properties: {
    bundle: { type: "string" },
    new_tests: { type: "integer" },
    covers: {
      type: "string",
      description: "source files/areas the tests target",
    },
    diagnostic_tests: {
      type: "integer",
      description:
        "negative/error-path DIAGNOSTIC_TESTs (validate locally even without FileCheck)",
    },
    unverified: {
      type: "integer",
      description: "tests tagged characterization-unverified=true",
    },
    findings: { type: "integer" },
    finding_ids: { type: "array", items: { type: "string" } },
    unreachable_noted: {
      type: "integer",
      description: "gaps recorded as unreachable/defensive",
    },
    notes: { type: "string" },
  },
  required: ["bundle", "new_tests"],
};

const PROMPT = (
  c,
) => `You are creating WHITE-BOX CHARACTERIZATION tests for a NEW bundle in the third generated-test tree. These target uncovered compiler code and pin its CURRENT observed behaviour (NOT spec). Read and follow the contract exactly.

NEW BUNDLE: \`docs/generated/tests/coverage/${c.bundle}/\` (create it: README.md + .slang files)
TARGET SOURCE: ${c.files}
REACHABLE-INPUT HINT: ${c.hint}
REPO ROOT: /home/jvepsalainen/workspace/jvepsalainen-nv/slang (branch 2026-06-emission-fanout-gate)
slangc: build/RelWithDebInfo/bin/slangc ; slangi: build/RelWithDebInfo/bin/slangi

READ FIRST (the authority for this tree):
- docs/generated/tests/coverage/METHODOLOGY.md  <-- THE contract for this suite
- docs/generated/tests/_meta/prompts/_common.md (for //META block shape, directives, diagnostic-test rules, finding format)
Then READ THE TARGET SOURCE FILE(S) above. This is white-box: you ARE allowed (only in this tree) to read the compiler source and choose tests to exercise its uncovered branches.

METHOD:
1. Read the target source. Identify CLI-reachable features / branches / error-paths that a normal slangc (or slangi) invocation can drive but that look under-tested. Focus on the hint area.
2. For each, write a test that drives real input through slangc/slangi and CLASSIFY the actual result:
   - clean expected output -> pin the CHECK to the REAL observed output (verbatim; tight on the distinguishing token, wild on incidental ids). If you cannot confirm the output is *correct* (vs a spec), add \`//META: characterization-unverified=true\`.
   - documented/clean diagnostic -> //DIAGNOSTIC_TEST pinning the exact E#### (these validate locally even without FileCheck — prefer them for error paths).
   - crash / abort / internal-error / malformed -> file a finding under docs/generated/tests/_meta/findings/<slug>.yaml; do NOT write a passing test.
3. Each test's //META block: standard keys PLUS \`//META: intent=characterization\` and \`//META: covers=<target source file>\`. NO doc_ref required (this tree is exempt).
4. Record gaps you judged unreachable/defensive in the README \`## Unreachable gaps\` (do not chase them). If a reachable behaviour really should be in the language reference, add a \`## Doc gaps observed\` row.

HARD RULES: never assume output — run the tool and read it; every new .slang must compile/run cleanly (slangc/slangi exit 0, or a clean expected diagnostic); pin CHECKs verbatim from real output. FileCheck is absent locally so SIMPLE filecheck tests report "ignored" on verify (CI validates) — but DIAGNOSTIC_TESTs and slangi INTERPRET runs DO validate locally, so prefer those where they fit. Do NOT git commit.

README.md for the bundle: front-matter per _common.md (generated/model/generated_at/source_commit/warning; source_doc may be the target source file path), then sections: ## Intent, ## Functional coverage (one row per test: what it pins + the covers= target), ## Unreachable gaps, ## Doc gaps observed.

Return the structured result: bundle, new_tests, covers, diagnostic_tests count, unverified count, findings (+finding_ids), unreachable_noted count, one-line notes (incl. whether the source was clear enough to target).`;

phase("Whitebox");
log(
  `White-box sweep over ${CLUSTERS.length} reachable source-area clusters (autodiff included; emit-llvm/reflection/ast-print excluded as unreachable)`,
);
const results = await parallel(
  CLUSTERS.map(
    (c) => () =>
      agent(PROMPT(c), {
        label: `whitebox:${c.bundle}`,
        phase: "Whitebox",
        schema: RESULT_SCHEMA,
      }),
  ),
);
const ok = results.filter(Boolean);
const totalNew = ok.reduce((s, r) => s + (r.new_tests || 0), 0);
const totalFind = ok.reduce((s, r) => s + (r.findings || 0), 0);
const totalUnver = ok.reduce((s, r) => s + (r.unverified || 0), 0);
log(
  `White-box done: +${totalNew} tests across ${ok.length} bundles; ${totalUnver} characterization-unverified; ${totalFind} findings`,
);
return { bundles: ok.length, totalNew, totalUnver, totalFind, results: ok };
