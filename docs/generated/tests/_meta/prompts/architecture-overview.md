# Prompt: docs/generated/tests/architecture/overview/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/architecture/overview/`,
anchored to
[`docs/generated/design/architecture/overview.md`](../../../docs/generated/design/architecture/overview.md).

Audience: nightly CI. The bundle exercises the architectural
introduction of Slang — what `slangc` produces, what translation
units and entry points are, what targets mean, and the existence of
the `Module` / `IComponentType` abstraction. Most of the doc is
**API-level** (`ISession`, `IGlobalSession`, `IModule`,
`IComponentType`, `Linkage`, `EndToEndCompileRequest`) and is **not
observable through `slang-test`**. The bundle therefore deliberately
defers most of the doc to `## Untested claims` and
keeps the test count small: **5–10 tests** is correct.

## The translation rule: API claims to slangc-observable behaviors

`architecture/overview.md` describes the COM-style public API and
the internal C++ objects (`Session`, `Linkage`,
`TranslationUnitRequest`, `EntryPointRequest`, `TargetRequest`,
`FrontEndCompileRequest`, `BackEndCompileRequest`,
`EndToEndCompileRequest`, `Module`, `IComponentType`).
`slang-test` runs `slangc` and its sibling tools as command-line
programs. So a claim such as "`Module` implements `IModule`" is not
testable through the agentic runner; only the claim's **observable
consequence at the command-line interface** is. Concretely:

- **Testable** ⇔ "if the doc claim were false, the
  `slangc`-driven compilation we wrote would change in a way the
  test directive reports (emit text differs, diagnostic fires,
  compile fails)."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class is allocated, which COM interface a pointer implements, or
  which header declares a method."

Here is the split for this doc:

### Observable claims (write tests for these)

- **Slang accepts entry points by name and stage**
  (`#compilation-request-lifecycle` — `EntryPointRequest` is "a
  function name plus a pipeline stage to compile"). Verify with
  `//TEST:SIMPLE...-entry foo -stage compute` and FileCheck the
  emitted text for that entry's body. Use distinct entry names in
  the same file to confirm `-entry` selects the right one.
- **Slang emits to multiple target languages from one source**
  (`#purpose` — "Slang is a shading-language compiler. Its job is
  to take Slang … source code and produce target code for a
  graphics or compute toolchain — DXIL, SPIR-V, GLSL, Metal …,
  WGSL, C++, CUDA…"). Verify by running the **same source file**
  through several `-target` directives and FileChecking
  target-specific syntax in each. This is the canonical
  multi-backend claim for the doc.
- **`-stage <stage>` selects the pipeline stage**
  (`#compilation-request-lifecycle` — `EntryPointRequest` carries
  the stage). Verify with a vertex-vs-compute split where the
  stage choice changes the emitted code in an observable way (a
  vertex-only intrinsic or layout attribute appears).
- **Slang inputs accept HLSL-compatible syntax**
  (`#purpose` — "Slang (and HLSL-compatible) source code").
  Verify by compiling an HLSL-syntax snippet (e.g.
  `cbuffer { … }`) to any text target and FileCheck the emitted
  result. The HLSL frontend's acceptance is the observable.
- **A `Module` is the front-end output for a translation unit; an
  entry point is exposed to clients via the standard `-entry`
  flag** (`#compilation-request-lifecycle`). Observable through
  the cross-section "we asked for `foo` and got `foo` in the
  emit".

### Not testable through slang-test (do NOT write tests for these)

- That `Session`, `Linkage`, `TranslationUnitRequest`,
  `EntryPointRequest`, `TargetRequest`, `FrontEndCompileRequest`,
  `BackEndCompileRequest`, `EndToEndCompileRequest` exist as C++
  classes with the relationships the doc describes.
- That `IGlobalSession` is the process-wide singleton and
  `ISession` is the per-compile-session interface.
- That `Module` implements `IModule`, or that a composite presents
  as `IComponentType`.
- That `Linkage` bundles search paths, preprocessor macros, target
  settings, and a source manager.
- That `slangc`, `libslang`, and `slang-rt` are the three primary
  build artefacts (a build-system fact, not a slangc behavior).
- The "HLSL inputs go one-per-unit / Slang inputs all together"
  rule — this is an internal `TranslationUnitRequest` packing
  detail; the user-visible effect (declarations in different
  Slang files in the same compile see each other) is testable but
  belongs better in a pipeline doc bundle. The architecture-doc
  claim itself describes the C++ object, not a user-surface
  behavior.
- The file-tree layout (`source/slang/`, `source/core/`,
  `source/compiler-core/`, `prelude/`, `tools/`, `external/`,
  `extras/`, etc.) — directory-structure facts, not slangc
  behaviors.
- The build-time code generation (`FIDDLE(...)`, Lua tables,
  `slang-ir-insts.lua`, `slang-diagnostics.lua`) — build-system
  facts.
- The standard-library packaging (`slang-core-module`,
  `slang-glsl-module`, `standard-modules`, `core.meta.slang`,
  `hlsl.meta.slang`, `diff.meta.slang`, `glsl.meta.slang`) — file
  inventory; not a slangc-runtime behavior.
- The downstream-compiler shims (`slang-llvm`, `slang-glslang`,
  `slang-dispatcher`) and the runtime / bindings tree
  (`slang-rt`, `slang-record-replay`, `slang-wasm`) — file
  inventory.
- The ABI / public-header rules from `CLAUDE.md` ("no enum
  reordering, no virtual-method changes mid-vtable, no removal").
  These are repo-policy invariants, not slangc behaviors.
- The reading-guide pointers to `module-map.md`,
  `dependency-graph.md`, `pipeline/overview.md`,
  `cross-cutting/*`, `syntax-reference/*` — navigation, not
  behavior.

If you find yourself thinking "this would verify that the C++
class `Module` is instantiated", stop — that is an API-shaped
probe in disguise. Re-read the doc and find the slangc-observable
consequence, or record the claim under
`## Untested claims`.

## Required structure

1. `README.md` with the structure named in `_common.md`. The
   `## Untested claims` section here doubles as the
   place to list API-shaped and file-tree claims that aren't
   observable via slangc at all (not literally GPU-bound, but
   identical in spirit: out of reach of the runner). **Expect this
   section to be long** — most of the doc lives here.
2. **5 to 10** `.slang` test files. Quality matters more than
   quantity; the doc has only a handful of slangc-observable
   claims, and inflating the bundle with low-value repetition (the
   same entry-point selection retested seven ways) is a contract
   violation. If a single test covers the claim at every feasible
   target, that *is* the right shape — see "Exercise as many
   backends as the claim allows" in `_common.md`.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/architecture/overview.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/pipeline/overview.md`
- `docs/generated/design/pipeline/01-lex-preprocess.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm that a claim in the doc
corresponds to actual command-line behavior (e.g. which option
flag drives the `EntryPointRequest`, the spelling of a target
name). You may **not** mine them for behavioral claims that the
doc does not make, and you may **not** write tests that probe
internal class identity.

- `source/slang/slang-compile-request.h`
- `source/slang/slang-compile-request.cpp`
- `source/slang/slang-module.h`
- `source/slang/slang-module.cpp`
- `include/slang.h`

## Test directives

The slangc-observable claims here are mostly about
**target-dependent emit** ("a single source compiles to N target
languages") or **stage / entry selection** at the command line.
So:

- **Multi-target tests are appropriate here** and required for the
  "Slang compiles to multiple targets" claim. The right shape is
  one `.slang` file with multiple `//TEST:SIMPLE` directives, one
  per text-emit target where the claim is observable: `hlsl`,
  `glsl`, `spirv-asm`, `metal`, `wgsl`, `cuda`, `cpp`. Use distinct
  `CHECK` patterns per directive only when a target's syntax
  differs in a meaningful way; otherwise share a single pattern
  that catches the common observable.
- `//TEST:SIMPLE(filecheck=CHECK):-target <t> -entry <name> -stage <stage>`
  is the dominant directive. Specify `-entry` and `-stage`
  explicitly so the test is anchored to the claim that those
  flags exist and behave as the doc describes.
- `//TEST:INTERPRET(filecheck=CHECK):` is fine for a claim that is
  pre-backend (e.g. an HLSL-syntax acceptance test that compiles
  and executes), but most of this bundle's observable claims are
  backend-flavoured.
- Do not use `DIAGNOSTIC_TEST` unless the doc explicitly makes a
  rejection claim. (The architecture overview makes no such
  claims; if you reach for `DIAGNOSTIC_TEST`, you are probably
  inventing.)
- Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `architecture/overview.md` (or one of the listed secondary
      docs).
- [ ] No test asserts the C++ identity of a compile-request,
      module, or component-type object.
- [ ] No test depends on a GPU.
- [ ] The "Slang compiles to multiple targets" claim is exercised
      by **at least one test that runs against more than one
      text-emit target**, not by N near-duplicate single-target
      tests.
- [ ] Tests use `-entry` / `-stage` explicitly where the claim is
      about those flags.
- [ ] `## Untested claims` in `README.md` is long.
      The doc is overwhelmingly about API / file-tree facts that
      are not observable through `slang-test`; the section should
      itemise the API claims, the directory-layout claims, the
      build-system claims, and the ABI-policy claims, so a future
      reader can see at a glance why the bundle is small.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch
      at `slang-compile-request.cpp:NNNN`", stop and re-read the
      doc.
