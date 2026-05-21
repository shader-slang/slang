# Prompt: tests-agentic/architecture/module-map/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/architecture/module-map/`,
anchored to
[`docs/llm-generated/architecture/module-map.md`](../../../docs/llm-generated/architecture/module-map.md).

Audience: nightly CI. The bundle exercises the source-tree
**module-map** doc — a lookup table that enumerates subdirectories
of `source/` (and `prelude/`) and the files inside them, with
one-line responsibility notes. The doc's purpose is "where is X
implemented?" Almost everything in it is a **file-tree fact** about
the repository, not a behavior observable through `slang-test`.

This is the sister doc to `architecture/overview.md`. The same
translation rule applies: **directory layout and file-name claims
are not slangc-observable**. The bundle is intentionally **small**
(5–10 tests) and the `## Out of scope (no-GPU runner)` section is
**large** — it itemises every subsystem the doc describes that is
not reachable from `slangc`'s command line.

If after three writing attempts you cannot find an additional
testable claim, **stop**. The doc is genuinely thin on
slangc-observables; padding with low-value tests is a contract
violation.

## The translation rule: file-tree claims to slangc-observable behaviors

`architecture/module-map.md` lists files and the C++ identity of
their contents (e.g. "`slang-lower-to-ir.cpp` walks the checked
AST and emits IR via `IRBuilder`"). The agentic runner cannot see
which C++ file a code path lives in. It only sees what `slangc`
emits, accepts, or rejects on the command line.

- **Testable** ⇔ "a *named subsystem* in the doc has a CLI
  consequence that would change if that subsystem were missing —
  `slangc` could not emit a target, could not accept a source
  form, could not load a built-in module, could not write a
  particular prelude include."
- **Not testable through slangc** ⇔ "a claim about which
  directory contains a file, which file has which prefix, which
  C++ class lives where, or which Lua table generates which
  header."

### Observable claims (write tests for these)

- **`source/slang/` houses a multi-backend emit dispatcher**
  (`#code-emission` — "Emit dispatcher selects backend per
  `TargetRequest`"; per-backend rows for `slang-emit-hlsl`,
  `-glsl`, `-spirv`, `-metal`, `-wgsl`, `-cuda`, `-cpp`). Verify
  with a single source compiled against several `-target`s; the
  presence of target-specific syntax in each emit is the
  observable consequence of the dispatcher's existence.
- **`source/slang-core-module/` ships a core module embedded into
  `libslang`** (`#sourceslang-core-module-embedded-core-module`).
  Verify by using a built-in core-module type (`int4`, `float3`,
  intrinsics) without any `import`; the program must compile and
  run on `slangi`. If the embedded core module were missing, the
  type would be undefined.
- **`source/slang-glsl-module/` ships a GLSL module embedded into
  `libslang`** (`#sourceslang-glsl-module-embedded-glsl-module`).
  Verify by `import glsl;` in Slang code (the embedded module
  must be resolvable). Optionally also verify `-allow-glsl`
  accepts GLSL-syntax input — that flag is what makes the GLSL
  frontend module observable.
- **`prelude/` ships one prelude per textual target family**
  (`#prelude-per-target-prelude-headers`). Verify by FileChecking
  the CUDA / CPP / Torch emit for the literal include of
  `slang-cuda-prelude.h` / `slang-cpp-prelude.h` /
  `slang-torch-prelude.h` — the emitted code contains the prelude
  include because the prelude file exists and is named exactly as
  the doc says.
- **`source/slangc/` is the command-line driver**
  (`#other-source-subdirectories`). Trivially observable: the
  fact that any test in this bundle runs at all is evidence; the
  `-get-module-info` round-trip is a tighter probe — write a
  module out, ask `slangc` to read it back, and FileCheck the
  result.

### Not testable through slang-test (do NOT write tests for these)

- The full per-file inventory of any subdirectory. The doc lists
  ~150 file names; that the file with that exact name exists is
  a build-system observation, not a `slangc` behavior.
- The C++ identity of any class named in the doc (`Token`,
  `SourceManager`, `IRBuilder`, `IRInst`, `Linkage`, `Session`,
  `Module`, …). C++ identity is invisible to the CLI.
- The file-name prefix conventions (`slang-ast-*`,
  `slang-check-*`, `slang-ir-*.cpp`, `slang-emit-*`,
  `slang-serialize*`, …). Pure naming convention.
- The "roughly 300 files implement IR passes" count, the
  pass-category groupings (cleanup, specialization,
  differentiation, layout, validation, …), and the per-pass
  filenames. The categorisation lives in the pipeline doc; even
  there, individual passes are not separately observable.
- The Lua-table-driven catalog (`slang-ir-insts.lua`,
  `slang-diagnostics.lua`, FIDDLE generation under
  `build/source/slang/fiddle/`). Build-system facts.
- The cross-cutting "see the dedicated doc for X" pointers
  (diagnostics, capability system, profiles, serialization).
  Navigation, not behavior.
- The "Other source/ subdirectories" inventory (`slang-llvm`,
  `slang-glslang`, `slang-dispatcher`, `slang-rt`,
  `slang-record-replay`, `slang-wasm`). The fact that slangc can
  delegate to LLVM or glslang is observable through `-target`,
  but the existence of the *bridge directory* is not. The
  `slang-record-replay` and `slang-wasm` directories ship code
  that is invoked outside `slangc`; the runner cannot reach
  them.
- `source/slang/diagnostics/` and the diagnostic catalog
  organisation. The text of any one diagnostic is observable via
  `DIAGNOSTIC_TEST`, but the *catalog file structure* is not.
- The standard-modules tree (`source/standard-modules/`, the
  `neural/` standard module). Its loading is gated on
  `-experimental-feature` and the doc does not assert a CLI
  behavior; if you write a test, anchor it under a future
  standard-modules bundle instead.

If you find yourself thinking "this would verify that
`slang-lower-to-ir.cpp` is the file that lowers", stop — that is
a file-identity probe in disguise. Re-read the doc and find the
slangc-observable consequence of *the subsystem named*, not the
file naming it.

## Required structure

1. `README.md` with the structure named in `_common.md`. The
   `## Out of scope (no-GPU runner)` section here doubles as the
   place to list file-tree, prefix-convention, and Lua-table
   claims that are not observable via slangc at all. **Expect
   this section to be long** — most of the doc lives here.
2. **5 to 10** `.slang` test files. The doc has only a handful
   of slangc-observable consequences; the right shape is one
   test per consequence, with multi-target directives where the
   consequence is target-spanning (the emit dispatcher) and a
   single-directive test where the consequence is per-target
   (one prelude include per target).

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/architecture/module-map.md`

Secondary citations are not needed for this bundle. If you would
cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm that a claim in the doc
corresponds to actual command-line behavior. You may **not**
mine them for behavioral claims that the doc does not make.

- `include/slang.h`
- `source/slangc/slangc-main.cpp`
- `source/slang/slang-emit.cpp`

## Test directives

The slangc-observable claims here are about **target-dependent
emit** (the dispatcher exists), **built-in module availability**
(core / GLSL modules are embedded), **prelude includes** in the
emitted text, and **`-get-module-info`** round-trip. So:

- The dispatcher claim is the canonical multi-target shape: one
  `.slang` file with `//TEST:SIMPLE` directives across all
  feasible text-emit targets.
- The core-module claim is pre-backend; `//TEST:INTERPRET` is
  ideal.
- The GLSL-module claim has two faces: `import glsl;` resolving
  is testable via INTERPRET or a `-target glsl` filecheck;
  `-allow-glsl` is testable by compiling a GLSL snippet to GLSL
  or SPIR-V text.
- The prelude-inventory claim is per-target text emit
  (`-target cuda`, `-target cpp`); FileCheck for the literal
  `#include "...prelude.h"` substring.
- The slangc-as-driver claim is best probed by
  `-get-module-info` on a `.slang-module` produced by the same
  invocation that read it back.
- Do not use `DIAGNOSTIC_TEST` for this bundle. The doc makes
  no rejection claims; if you reach for it, you are inventing.
- Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `architecture/module-map.md`.
- [ ] No test asserts the C++ identity of a file, class, or
      directory.
- [ ] No test depends on a GPU.
- [ ] The dispatcher claim is exercised by **at least one test
      that runs against more than one text-emit target**.
- [ ] `## Out of scope (no-GPU runner)` in `README.md` is long
      and itemises file-tree claims, prefix conventions, Lua
      tables, IR-pass enumeration, and cross-cutting subsystem
      pointers.
- [ ] No test was written by inspecting an uncovered source
      line. If you find yourself thinking "this would cover the
      branch at `slang-emit.cpp:NNNN`", stop and re-read the
      doc.
