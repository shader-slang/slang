# Prompt: docs/generated/tests/regression/pipeline/05-ir-passes/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/regression/pipeline/05-ir-passes/`,
anchored to
[`docs/generated/design/pipeline/05-ir-passes.md`](../../../design/pipeline/05-ir-passes.md).

Audience: nightly CI. The bundle exercises the **IR-pass catalog** —
the set of transformations that run between AST → IR lowering
([04-ast-to-ir](../../../design/pipeline/04-ast-to-ir.md))
and code emission
([06-emit](../../../design/pipeline/06-emit.md)). The
source doc groups ~325 `slang-ir-*.cpp` files into nine categories
(Linking and validation, SSA construction / cleanup, Specialization
and generics, Differentiation, Type and value legalization, Inlining
and call-graph, Entry-point and parameter handling, Layout and binding,
Loop transformations, Target-specific lowering, Instrumentation). Per-
category tests verify either a per-pass observable effect (IR-dump
diff before vs after the pass), or the _consequence_ of the pass on
emit text for the targets that the pass runs for.

This bundle is the largest in the suite by design — `size_cap_files`
is 100. Aim for 20–40 tests covering most categories. Quality matters
more than line-coverage: a pass that is too deep to observe through
slangc CLI must be dropped and recorded under
`## Untested claims`. Do **not** mine source files for
behavioral claims that the doc does not make.

## The translation rule: claims to observations

The source doc's per-category tables are explicit: "the orchestrator
is `linkAndOptimizeIR` … this document reflects categories, not
order." The testable consequences of each row are:

- **"Pass X removes opcode Y"** — compile with
  `-dump-ir-before X -dump-ir-after X -target <text-target> -o /dev/null`
  and FileCheck that Y appears in `### BEFORE X:` but not in
  `### AFTER X:`.
- **"Pass X is target-specific to target T"** — compile to a text
  target that the doc lists for the pass and FileCheck the emit (or
  the IR dump) for the rewrite Y; compile to a target for which the
  pass does NOT run and FileCheck the emit for the original opcode.
- **"Pass X enforces invariant Z"** — write source that violates Z
  and FileCheck the diagnostic via `DIAGNOSTIC_TEST` (e.g.
  recursion check, missing-return).
- **"Pass X has a user-observable consequence in emit"** — for many
  passes the cleanest observation is in the target text rather than
  in the IR dump (collect-global-uniforms creating a `GlobalParams`
  struct on HLSL/CUDA, byte-address legalize introducing
  `asuint`/`Load`-shaped accesses, optional/result lowering
  producing a flat struct on every target). Use multi-target SIMPLE
  for these.

### Observable claims (write tests for these)

Per category, anchored to the headings inside `05-ir-passes.md`:

#### `#linking-and-validation`

- **Check recursion** (`slang-ir-check-recursion.cpp`) diagnoses
  unsupported recursion. Negative test: a self-recursive function
  raises an error. (Wider regular language tests live under
  `tests/`; here we anchor the claim "this validation pass exists
  and rejects recursion".)
- **Missing-return** (`slang-ir-missing-return.cpp`) warns when a
  non-void function lacks a return on every control-flow path.
  Negative test: warning is observed.
- **Use of uninitialized values** (`slang-ir-use-uninitialized-values.cpp`)
  or **Detect uninitialized resources**
  (`slang-ir-detect-uninitialized-resources.cpp`) diagnose
  uninitialized usage. Skip if no clean .slang surface produces the
  diagnostic at this stage rather than at semantic check.

#### `#ssa-construction-and-basic-cleanup`

- **DCE** (`slang-ir-dce.cpp`) removes a top-level helper function
  that is never called and never exported. Positive test: source
  defines `int helper(...)` not called from `main`; the emitted
  HLSL/SPIR-V/GLSL text does NOT contain `helper`. Use
  `CHECK-NOT: helper`.
- **Single return** (`slang-ir-single-return.cpp`) and the **eliminate
  multi-level break** pass change CFG shape — observe in the emit
  text only if a multi-target diff is visible (these are mostly
  internal). Prefer skipping if no clean observation surface exists.
- **Cleanup void** / **Strip default construct** / **Strip
  legalization insts** — internal scaffolding; observe via emit
  text only if a clean post-pass token is named in the doc.
- **Init local var** (`slang-ir-init-local-var.cpp`) — observable
  on targets that require explicit init: SPIR-V text shows
  `OpStore` initializing a local. (Cite the section heading, not a
  particular target marker, if you cannot anchor to the doc's text.)

#### `#specialization-and-generics`

- **Specialize** (`slang-ir-specialize.cpp`) substitutes generic
  parameters with concrete types. Observation: compile a generic
  call to a concrete type and FileCheck the IR dump
  `### BEFORE specializeModule:` shows `specialize(%generic, T)`
  while the final emit shows the specialized body (no `generic`
  remaining). Pair with the upstream `cross-cutting/ir-instructions`
  bundle's `specialize` opcode test.
- **Lower dynamic-dispatch insts** + **Bind existentials** —
  observable when an interface-typed local is initialized from a
  concrete type; emit shows the resolved call site rather than
  `lookup_witness`. Skip if the test cannot be anchored to a
  doc-stated claim.

#### `#type-and-value-legalization`

This is the richest category for observable consequences.

- **Lower optional type** (`slang-ir-lower-optional-type.cpp`):
  source with `Optional<int>` returns lowers to a struct with
  `value` and `hasValue` fields on HLSL/GLSL/CUDA. CHECK for
  `hasValue` on every applicable text target.
- **Lower tuple types** (`slang-ir-lower-tuple-types.cpp`): a
  `Tuple<int,int>` flattens — the emit shows separate `_0` / `_1`
  accesses (or fully inlined values), not a tuple type.
- **Lower buffer element type** + **Wrap structured buffers**
  affect how `RWStructuredBuffer<T>` appears on each target. The
  `cross-cutting/ir-instructions` bundle covers the
  `rwstructuredBufferGetElementPtr` IR opcode; this bundle
  observes the per-target emit shape.
- **Lower out parameters** / **Lower l-value cast** affect ABI of
  out parameters. Observe in emit text where applicable.

#### `#inlining-and-call-graph`

- **Inline** (`slang-ir-inline.cpp`): a `[ForceInline]` function
  call is inlined into the caller, observable as the body of the
  callee appearing in `main`'s emit rather than a `helper(...)`
  call.
- **DLL export** (`slang-ir-dll-export.cpp`): exported entry-point
  decoration affects the SPIR-V `OpEntryPoint` shape (already
  covered by `cross-cutting/ir-instructions`).

#### `#entry-point-and-parameter-handling`

- **Entry-point decorations** (`slang-ir-entry-point-decorations.cpp`):
  `[numthreads(...)]` is lowered into the entry-point function's
  emit as the target-specific marker (`[numthreads]` on HLSL,
  `layout(local_size_x = ...)` on GLSL, `OpExecutionMode ... LocalSize`
  on SPIR-V). This is also exercised in `cross-cutting/ir-instructions`
  via the IR side and in `pipeline/06-emit` via the per-target side;
  this bundle adds the **"the entry-point pass causes this"** angle —
  multi-target SIMPLE with `numthreads` / `local_size_x` /
  `LocalSize` patterns.
- **Entry-point uniforms** + **Collect global uniforms**
  (`slang-ir-collect-global-uniforms.cpp`): a free-floating
  `uniform int a;` at module scope is collected into a
  `GlobalParams_0` cbuffer on HLSL and into a `block_GlobalParams_0`
  uniform block on GLSL. CHECK that the cbuffer / uniform block /
  CUDA `SLANG_globalParams` exists.

#### `#layout-and-binding`

- **Translate global varying var** + entry-point varying param
  rewriting: an `SV_DispatchThreadID` input is consumed differently
  per target. Observation is at emit text. (Coverage of this is
  weaker because the bundle ought not duplicate `pipeline/06-emit`;
  skip unless the doc explicitly calls a per-pass effect.)

#### `#loop-transformations`

- **Loop unroll** (`slang-ir-loop-unroll.cpp`): `[unroll]` for a
  loop of compile-time-known iteration count causes the loop body
  to appear unrolled in the emit (no loop construct remains). The
  HLSL emit shows `[unroll]` in a literal `for(;;)` form (the HLSL
  emitter intentionally preserves the loop and tags it). On GLSL /
  CUDA / SPIR-V the loop is fully unrolled by Slang itself when the
  bound is constant. Pin to one target where the observation is
  unambiguous.

#### `#target-specific-lowering`

- **GLSL legalize** (`slang-ir-glsl-legalize.cpp`) rewrites the
  resource shape — a `RWStructuredBuffer<T>` becomes
  `layout(std430, binding = N) buffer { T _data[]; }`. Use a
  single-target GLSL test that checks for the `_data[` token.
- **SPIR-V legalize** (`slang-ir-spirv-legalize.cpp`) ensures
  `OpCapability Shader`, `OpEntryPoint GLCompute`, and the
  documented decoration shape are present in `-target spirv-asm`.
- **Metal legalize** (`slang-ir-metal-legalize.cpp`) introduces
  positional `[[buffer(N)]]` / `kernel` markers. Per `_common.md`,
  match `kernel` and `buffer(0)` as bare substrings — never as
  literal `[[kernel]]`, which FileCheck would parse as a regex
  variable.
- **WGSL legalize** (`slang-ir-wgsl-legalize.cpp`) inserts
  `@binding(N) @group(N)` and `@compute @workgroup_size(...)`.
- **CUDA immutable load** (`slang-ir-cuda-immutable-load.cpp`)
  wraps reads from uniform globals in `__ldg(&...)` on CUDA.
  Observation: CHECK for `__ldg`. Per `_common.md`, compound
  expressions over `uniform` operands on CUDA factor into
  temporaries; derive any binary expression you also want to
  observe from `SV_DispatchThreadID` not from uniforms.

#### `#instrumentation` and `#other-passes`

These are either gated on flags (`-trace-coverage-binding`) or
internal scaffolding (SPIR-V opcode snippets). Skip unless a
specific surface-observable claim is named.

### Not testable through slangc (record under `## Untested claims`)

- The **exact order** in which passes run for a given target — the
  doc explicitly defers to `linkAndOptimizeIR` and the per-target
  pipelines under `target-pipelines/`. A textual ordering would be
  brittle and is not a doc claim here.
- Per-pass C++ helper functions and class structures (e.g. the
  `IRBuilder` usage inside each pass). Internal API.
- Whether a hoistable instruction floats to the outermost dominator
  — the IR dump shows post-hoist text, not the hoisting decision.
- IR pass **utilities** (`Clone`, `Dominators`, `Util`, `Insts info`,
  `Insts stable names`) — they are not transformations and have
  no observable effect of their own.
- The pre-link region documented in
  [04b-pre-link-passes.md](../../../design/pipeline/04b-pre-link-passes.md);
  if a claim is about an explicitly pre-link pass, it belongs in
  that bundle's prompt, not here.
- Coverage instrumentation flags (`-trace-coverage-binding`,
  `-trace-coverage-reserved-space`) — these are command-line
  surface; the agentic bundle does not exercise them.
- Differentiation passes (`slang-ir-autodiff-*.cpp`) require a
  `[Differentiable]` function and a `bwd_diff` / `fwd_diff` call
  to produce observable output; the doc states the pass family
  but does not specify the user-observable consequence at a level
  this bundle can anchor cleanly. Skip and record as out-of-scope;
  cite the design doc gap.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for unobservable
   items.
2. 20 to 40 `.slang` test files. Many tests are single-target
   `-dump-ir` observations; some are multi-target SIMPLE emit
   observations; a small number are `DIAGNOSTIC_TEST` for the
   validation passes that emit diagnostics. The `size_cap_files`
   cap is 100; do not exceed it.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/pipeline/05-ir-passes.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off or the test specifically observes
the hand-off boundary):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`
- `docs/generated/design/pipeline/06-emit.md`
- `docs/generated/design/ir-reference/index.md`
- `docs/generated/design/target-pipelines/spirv.md`
- `docs/generated/design/target-pipelines/hlsl.md`
- `docs/generated/design/target-pipelines/metal.md`
- `docs/generated/design/target-pipelines/wgsl.md`
- `docs/generated/design/target-pipelines/cuda.md`
- `docs/generated/design/target-pipelines/index.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm a pass-name spelling (the
exact name as it appears in `-dump-ir-after <name>`) or an opcode
spelling. You may **not** mine them for behavioral claims that the
doc does not make.

- `source/slang/slang-ir-*.h`
- `source/slang/slang-ir-*.cpp`
- `source/slang/slang-emit.cpp` (specifically `linkAndOptimizeIR`,
  for the orchestration sequence; do not test the ordering itself)

## Test directives

Two primary modes, plus diagnostic tests for validation passes.

1. **Pass-effect observation via `-dump-ir-before/-dump-ir-after`**:

   ```
   //TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-before <pass> -dump-ir-after <pass> -o /dev/null -stage compute -entry main
   ```

   Anchor `CHECK` patterns to `### BEFORE <pass>:` and
   `### AFTER <pass>:` headers. Pair `CHECK-LABEL: ### BEFORE
<pass>:` with a `CHECK: <opcode>` that should be present, and
   `CHECK-LABEL: ### AFTER <pass>:` with a `CHECK-NOT: <opcode>`
   that should be removed (or vice versa). Per `_common.md`, use
   `-o /dev/null` so target text does not mix with the IR dump.

2. **Pass-consequence observation in emit text**:

   ```
   //TEST:SIMPLE(filecheck=HLSL):-target hlsl  -stage compute -entry main
   //TEST:SIMPLE(filecheck=GLSL):-target glsl  -stage compute -entry main
   //TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
   //TEST:SIMPLE(filecheck=CUDA):-target cuda  -stage compute -entry main
   ```

   Per-target CHECK patterns go in comments tagged with the
   matching `filecheck=<NAME>` prefix. Apply the multi-backend rule:
   if a pass affects the emit on multiple targets, test on every
   feasible text target, not just one.

3. **Diagnostic tests** for the validation passes:

   ```
   //DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main
   ```

   Use only where the doc names the validation pass and the
   diagnostic is the surface-observable consequence. Place the
   carets on the column reported by the runner's "Suggested
   annotations" — do not hand-count.

Do not use any GPU-only directive.

## Lessons captured for IR-pass tests

These are in addition to the universal lessons in `_common.md`.

- **`-dump-ir-before/-dump-ir-after` accept the C++ pass entry
  function name** (e.g. `specializeModule`, `eliminateDeadCode`,
  `simplifyIR`). The header lines that appear are
  `### BEFORE <name>:` and `### AFTER <name>:`. If the name does
  not match, no header is emitted (the pass header silently does
  not appear; FileCheck then fails with "expected string not
  found"). Use the names that show up in a `-dump-ir` run on a
  representative source as the source of truth.
- **`simplifyIR` runs multiple times.** Targeting it produces
  multiple `### BEFORE simplifyIR:` / `### AFTER simplifyIR:`
  blocks. Use `CHECK-LABEL` to anchor at the first occurrence
  rather than asserting "the only".
- **Pre-link DCE removes locally-unused helpers before the post-
  link `eliminateDeadCode` runs.** A `unusedHelper` defined at file
  scope but never called is already gone in the first dumped stage.
  To observe a `BEFORE eliminateDeadCode` / `AFTER eliminateDeadCode`
  diff, the IR must contain something the post-link DCE removes —
  typically the easier mode is to observe the emit text not
  containing the helper.
- **`-dump-ir` produces a large preamble** of core-module imports,
  capability sets, differentiation glue, autodiff witness tables.
  Anchor at `func %main` or a user-named function or the explicit
  `### <stage>:` header.
- **HLSL preserves `[unroll]` literally in emit** even after the
  loop-unroll pass runs; the unroll is performed by the downstream
  DXC compiler, not by Slang. To observe a Slang-side unroll of a
  constant-bound loop, prefer GLSL or SPIR-V emit and check that
  no `for` keyword remains.
- **DCE observation by emit text** is the cleanest mode for the
  cleanup category. `CHECK-NOT: unusedHelper` on the final emit
  text works on every text target; do not try to observe DCE in
  the IR dump because the pre-link version has already removed it.
- **Collect-global-uniforms is observable on HLSL / GLSL / CUDA**:
  free-floating `uniform int a;` becomes a field of
  `GlobalParams_0` on HLSL, of `block_GlobalParams_0` on GLSL,
  and of `SLANG_globalParams` on CUDA. WGSL and Metal use their
  own layout. SPIR-V text shows it as the named global with
  `OpDecorate ... Binding`.
- **Lower-optional-type is observable on every C-like target**:
  `Optional<T>` becomes a struct with `value` + `hasValue` fields
  (`_slang_Optional_int_0` on HLSL). The shape token is
  `hasValue`.
- **CUDA `__ldg`** is the CUDA-immutable-load pass's footprint.
  CUDA reads of `uniform` globals appear as `__ldg(&...)` in the
  emit text. To observe a binary expression on CUDA, derive
  operands from `SV_DispatchThreadID` (CUDA factors `__ldg(&u)`
  into a temporary that splits compound expressions over uniforms).

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/05-ir-passes.md` (or one of the listed secondary
      docs).
- [ ] `-dump-ir`-based tests use `-target <text-target> -o /dev/null`
      per CLAUDE.md.
- [ ] Multi-target SIMPLE tests use a distinct `filecheck=<NAME>`
      label per target and per-target CHECK prefixes.
- [ ] No `[[...]]` literal in FileCheck patterns — use `kernel`,
      `buffer(0)`, `thread_position_in_grid` as bare tokens.
- [ ] CUDA arithmetic observations derive operands from
      `SV_DispatchThreadID` (or other thread/dispatch ID), not
      from `uniform` globals.
- [ ] Metal `[[buffer(N)]]` indices are positional — do not assert
      a specific index driven by HLSL `register(u3)` or
      `vk::binding(...)`.
- [ ] Validation-pass diagnostic tests use the runner's "Suggested
      annotations" for column positions.
- [ ] No test depends on a GPU.
- [ ] No test asserts on the order in which passes run.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `## Doc gaps observed` records claims the doc makes that
      were not testable here, especially the differentiation
      family and the long tail of legalization passes.
- [ ] `## Untested claims` records claims that are
      genuinely unobservable through any allowed directive
      (utilities, hoisting decisions, pass-ordering, coverage
      instrumentation flags).
