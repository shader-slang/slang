# Prompt: docs/generated/tests/regression/cross-cutting/targets/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/regression/cross-cutting/targets/`,
anchored to
[`docs/generated/design/cross-cutting/targets.md`](../../../design/cross-cutting/targets.md).

Audience: nightly CI. The bundle exercises the **target abstraction**
described in the source doc: that the compiler has multiple text-emit
back-ends, that the front-end's capability system rejects features the
selected target does not support, that `Profile` bundles a `Stage` +
`Family` + version with a capability set, and that target-specific
lowering surfaces in the emitted text.

The bundle is **multi-backend-heavy**: most testable claims are about
cross-target behaviour. The default is one `//TEST:SIMPLE` directive
per feasible text-emit target, plus `//DIAGNOSTIC_TEST` directives for
the "feature X is rejected on target Y" claims.

## The translation rule: claims to observations

`targets.md` describes:

- The **targets table** (`#targets`) — the set of `SlangCompileTarget`
  values grouped by emit backend. The observable consequence is: for
  every text-emit target the doc lists (HLSL / GLSL / SPIR-V / Metal /
  WGSL / CUDA / C++), `slangc -target <t>` produces target-shaped
  text.
- The **capability system** (`#capability-system`, `#vocabulary`,
  `#definition-forms`, `#runtime-representation`) — atoms populate
  target / stage keyholes; `+` is conjunction; `|` is disjunction;
  incompatible atoms (e.g. `hlsl + glsl`) cannot both apply. The
  user-facing surface is: features that require a capability the
  current target does not provide are rejected with a "use of
  undeclared identifier" or capability-mismatch diagnostic.
- The **profiles** (`#profiles`) — a profile pairs a `Stage` (compute /
  vertex / fragment / raytracing stages) with a `Family` and version.
  The user-facing surface is: `-profile <name>` parses and constrains
  emit; selecting an incompatible stage / target / profile triple is
  rejected.
- **How target choice affects IR** (`#how-target-choice-affects-ir`) —
  `[target]` / `[stage]` switches resolve against the active
  `TargetRequest`, and target-specific legalization passes shape the
  emitted text. The user-facing surface is: a `__target_switch` with
  per-target branches emits the branch matching `-target <t>` and the
  same `[shader("...")]` function emits a target-shaped entry-point
  marker.

### Observable claims (write tests for these)

- **Each documented text-emit target compiles**
  (`#targets`): one source emitted to HLSL / GLSL / SPIR-V / Metal /
  WGSL / CUDA / C++ each produces target-shaped text. (Smoke test.)
- **`SlangCompileTarget` text variants** (`#targets`): the SPIR-V
  group exposes both binary (`-target spirv`) and assembly
  (`-target spirv-asm`) text; the no-GPU runner observes the
  assembly form.
- **Stage atom + target atom interaction** (`#vocabulary`,
  `#definition-forms`): a `[shader("compute")]` entry point compiles
  on every backend that supports the compute stage (every text-emit
  target the doc lists supports `compute`).
- **Stage / target capability mismatch is rejected**
  (`#capability-system`): using a fragment-only feature like
  `discard` inside a compute entry point is rejected with a
  capability-mismatch diagnostic on HLSL.
- **Restrictive capability check upgrades capability warnings**
  (`#runtime-representation`): `-restrictive-capability-check`
  converts an "implicitly upgraded profile" warning into an error
  for a `__requireCapability(<atom>)` outside the active profile;
  `-ignore-capabilities` suppresses it. (This exercises the
  `Computing the join (intersection) of two capability sets`
  bullet.)
- **Profile string parses** (`#profiles`): `-profile sm_6_0` /
  `-profile glsl_450` / `-profile spirv_1_5` are accepted and the
  capability set they imply lets a vanilla compute shader compile.
- **Per-target lowering surfaces in emit**
  (`#how-target-choice-affects-ir`): a compute entry point emits the
  HLSL `[numthreads(...)]`, GLSL `layout(local_size_x = ...)`,
  SPIR-V `OpExecutionMode LocalSize`, Metal `kernel` /
  `thread_position_in_grid`, WGSL `@workgroup_size`, CUDA
  `extern "C" __global__`, and C++ `SLANG_PRELUDE_EXPORT` markers.
- **`__target_switch` resolves against the active target**
  (`#how-target-choice-affects-ir`): a function with per-target
  branches emits the body of the branch matching `-target <t>` on
  the corresponding text target.
- **Target-specific syntax is rejected on incompatible targets**
  (`#capability-system`): an explicit `__requireCapability(<atom>)`
  whose atom populates a different `target` keyhole is rejected
  when the active target does not match.

### Not testable through slangc text emit (record under `## Untested claims`)

- The **`.capdef` syntax** (`#definition-forms`) — the build-time
  generator runs once and the result is baked into the compiler; the
  user-facing observable is only "this atom is or is not satisfied",
  not the form of its declaration.
- **`Capability` / `CapabilitySet` C++ layout**
  (`#runtime-representation`) — internal data structure.
- **`slang-capability-generator` build behaviour** — a build-system
  step.
- The **`a3-02-reference-capability-atoms.md` doc-comment harvest**
  (`#auto-generated-reference`) — documentation tooling, not a
  compile-time observable.
- The **deprecated `SLANG_GLSL_VULKAN_*` aliases** (`#targets`) — the
  doc notes them but the active target group is `SLANG_GLSL`; we do
  not test deprecated aliases.
- **DXBC / DXIL / metallib / WGSL→SPIRV downstream paths** (`#targets`)
  — they require external compilers (FXC / DXC / Metal CLI / Tint)
  the no-GPU runner does not run.
- **Torch / LLVM / VM / Slang-round-trip targets** (`#targets`) — same
  reasoning as `pipeline/06-emit`'s out-of-scope list.
- The **`Adding a new target` checklist** (`#adding-a-new-target`) — a
  developer workflow, not a user-observable behaviour.
- The **per-target pass pipeline diagrams**
  (`#per-target-pass-pipelines`) — those belong to the
  `target-pipelines/*` bundles.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 10 to 20 `.slang` test files. The bundle is multi-backend-heavy:
   most tests carry multiple `//TEST:SIMPLE` directives, one per
   text-emit target where the claim is observable. Use distinct
   `filecheck=<NAME>` labels (e.g. `HLSL`, `GLSL`, `SPIRV`, `METAL`,
   `WGSL`, `CUDA`, `CPP`).
3. Mix:
   - **multi-target SIMPLE** tests (a feature accepted on multiple
     targets emits target-shaped text on each);
   - **`DIAGNOSTIC_TEST`** tests (a target / stage / capability
     mismatch is rejected with the documented diagnostic);
   - **`INTERPRET`** tests sparingly — most target claims are by
     definition target-specific, so `slangi` is rarely the right
     runner. Reserve it for **target-independent** observations
     (e.g. "every target supports the compute stage atom"; but the
     compute stage existing is more naturally a per-target emit
     observation).

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/cross-cutting/targets.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/generated/design/architecture/overview.md`
- `docs/generated/design/pipeline/06-emit.md`
- `docs/generated/design/target-pipelines/index.md`
- `docs/generated/design/target-pipelines/hlsl.md`
- `docs/generated/design/target-pipelines/spirv.md`
- `docs/generated/design/target-pipelines/metal.md`
- `docs/generated/design/target-pipelines/wgsl.md`
- `docs/generated/design/target-pipelines/cuda.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm the spelling of a capability
atom or a profile name. You may **not** mine them for behavioural
claims the doc does not make.

- `source/slang/slang-emit-*.h`, `slang-emit-*.cpp`
- `source/slang/slang-capability.h`, `slang-capability.cpp`
- `source/slang/slang-capabilities.capdef`
- `source/slang/slang-profile.h`, `slang-profile.cpp`
- `source/slang/slang-profile-defs.h`

## Test directives

Most claims are multi-target. The default for a positive emit test:

```
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda  -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp   -entry main -stage compute
```

For a negative (capability / profile mismatch) test:

```
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile sm_5_0 -entry main -stage compute -restrictive-capability-check
```

Avoid GPU-only directives. Reserve `INTERPRET` for the rare claim
that is genuinely target-independent.

## Lessons captured for target-stage tests

These add to the universal lessons in `_common.md`. Apply them ALL:

- **Metal `[[buffer(N)]]` is positional**, not driven by
  `vk::binding(N)` or HLSL `register(uN)`. When asserting Metal
  binding shape, accept any `buffer({{[0-9]+}})` rather than a
  specific N.
- **CUDA factors `__ldg(&uniform)` reads into temporaries.** A
  compound expression like `a + b` over two `uniform` globals
  becomes `__ldg(...)` + `__ldg(...)` + `... + ...` on separate
  lines on CUDA. To observe `+` directly on CUDA, derive operands
  from `SV_DispatchThreadID` or other locals, not `uniform`
  globals.
- **`__target_switch` must include a `default:` arm** (or be
  exhaustive over every active target keyhole) when feeding through
  `slangc` — a missing arm for an active target is itself a
  capability-mismatch diagnostic. Either cover every target the
  test uses or pair with `-target` directives only for the covered
  arms.
- **`-restrictive-capability-check` upgrades the implicit
  capability-upgrade warning to an error** for HLSL; without it the
  same source emits a warning. A diagnostic test for the
  "restrictive mode rejects" claim should pass the flag.
- **The diagnostic for a target/stage capability mismatch attaches
  to the entry-point function signature**, not the offending
  statement inside. The `DIAGNOSTIC_TEST` runner's "Suggested
  annotations" output is the source of truth for column positions.
- **Source-language claim mapping**: `-target hlsl` always emits
  HLSL text regardless of input source language. The `SourceLanguage`
  enum (`#targets`) is an input-side concept; do not write tests
  that probe input-side detection. The doc lists it as the input
  flavour of a translation unit, and our tests author Slang source.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `cross-cutting/targets.md` (or one of the listed secondary
      docs, and only when the primary doc hands off).
- [ ] Each multi-target `.slang` file carries `//TEST:SIMPLE`
      directives for every text-emit target where the claim is
      observable.
- [ ] CHECK patterns avoid raw `[[...]]` (FileCheck variable
      syntax) and use wildcards (`{{[0-9]+}}`, `{{.*}}`) for
      mangled identifiers.
- [ ] At least one test exercises a `DIAGNOSTIC_TEST` for a
      capability / profile / stage mismatch.
- [ ] No test depends on a GPU. Only SIMPLE text-emit,
      DIAGNOSTIC_TEST, and (rarely) INTERPRET are used.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch
      at `slang-capability.cpp:NNNN`", stop and re-read the doc.
- [ ] `## Untested claims` lists Torch / LLVM / VM /
      DXBC / DXIL / metallib / Tint downstream paths, `.capdef`
      grammar, the `Adding a new target` checklist, and the
      per-target pass diagrams (those belong to
      `target-pipelines/*`).
