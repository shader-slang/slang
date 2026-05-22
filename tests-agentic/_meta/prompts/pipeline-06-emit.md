# Prompt: tests-agentic/pipeline/06-emit/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/06-emit/`, anchored
to
[`docs/llm-generated/pipeline/06-emit.md`](../../../docs/llm-generated/pipeline/06-emit.md).

Audience: nightly CI. The bundle exercises the **code-emission stage**
of the Slang compiler: how a fully-legalized `IRModule` is turned into
per-target text or binary. The defining characteristic of this stage
is that it produces **per-target output** — so by construction this is
a **multi-backend-heavy** bundle. The default for a test here is to
attach one `//TEST:SIMPLE` directive per text-emit target the claim
is observable on, not one.

## The translation rule: claims to observations

`06-emit.md` describes:

- the **emit dispatcher** (`emitEntryPointsSourceFromIR`) that routes
  a compile to the right backend class per `TargetRequest`,
- the **per-target backends** (HLSL, GLSL, SPIR-V, Metal, WGSL, C++,
  CUDA, Torch, LLVM, VM, Slang round-trip) and their entry-point /
  resource markers,
- the **`SourceWriter`** abstraction and its `#line`-directive
  emission for cross-tool error mapping,
- the **operator precedence helper** that inserts only the parentheses
  needed to preserve semantics,
- the **preludes** that HLSL, CUDA, and C++ emitted code `#include`,
  in contrast to GLSL / Metal / WGSL which do not ship a `prelude/`
  header,
- the **dependency-file output** path (Make-style `.d` files).

The testable consequences are:

- **"Target T compiles and produces target-shaped text"** — compile
  the same Slang source to every feasible text-emit target and
  FileCheck for a target-distinctive marker. This is the dominant
  test pattern in this bundle.
- **"Target T has emit-stage marker M for construct C"** — write a
  test that puts construct C in source and FileCheck the per-target
  emit for the canonical marker (HLSL `[numthreads]` /
  `register(uN)`, GLSL `layout(local_size_x=...)` /
  `layout(std430, binding=N)`, SPIR-V `OpExecutionMode LocalSize` /
  `OpDecorate ... Binding`, Metal `kernel` /
  `thread_position_in_grid` / `[[buffer(N)]]`, WGSL `@compute` /
  `@workgroup_size` / `@binding(N) @group(N)`, CUDA `__global__` /
  `SLANG_globalParams`, C++ `SLANG_PRELUDE_EXPORT`).
- **"The SourceWriter emits `#line` directives"** — compile to a
  C-like text target and FileCheck for `#line` in the emitted source.
- **"The precedence helper inserts the minimum parentheses"** —
  write a source where one operator combination requires parens
  (e.g. `(a + b) * c`) and another does not (e.g. `a * b + c`);
  FileCheck that the parens-required case keeps them and the
  parens-not-required case drops them on every C-like target.
- **"Preludes are included by emitted code on the targets that ship
  a header prelude"** — compile to HLSL, CUDA, and C++; FileCheck
  for the per-target prelude / NVAPI-include / SLANG_PRELUDE marker.

### Observable claims (write tests for these)

- **Emit dispatcher routes per `TargetRequest`** to a per-target
  backend (`#emit-dispatcher`): the same source compiles successfully
  for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++ and each emits
  recognizable target text.
- **HLSL backend** (`#hlsl`) emits HLSL source with HLSL-shaped entry
  point (`[numthreads(...)]`) and HLSL register annotations
  (`register(uN)`).
- **GLSL backend** (`#glsl`) emits a GLSL source with `#version` and
  `layout(local_size_x = ...)` plus `layout(std430, binding = N)` for
  resources.
- **SPIR-V backend** (`#spir-v-direct`) emits SPIR-V assembly with
  `OpCapability Shader`, `OpEntryPoint GLCompute`, `OpExecutionMode
  ... LocalSize`, and `OpDecorate ... Binding N`.
- **Metal backend** (`#metal`) emits Metal source with
  `#include <metal_stdlib>` / `using namespace metal;` and `kernel`
  /`thread_position_in_grid` / `buffer(N)` markers.
- **WGSL backend** (`#wgsl`) emits WGSL with `@binding(N) @group(N)`
  resource markers and `@compute` / `@workgroup_size` entry-point
  markers.
- **C++ backend** (`#c`) emits C++ source that `#include`s the
  C++ shader prelude and exports a `SLANG_PRELUDE_EXPORT` entry
  point.
- **CUDA backend** (`#cuda`) emits CUDA source that `#include`s the
  CUDA prelude and exports an `extern "C" __global__` entry point.
- **`SourceWriter` emits `#line` directives** (`#source-writer-abstraction`)
  on at least the HLSL / GLSL / Metal / CUDA / C++ text targets so
  downstream compilers can map errors back to the original Slang
  source.
- **Operator-precedence helper inserts minimal parentheses**
  (`#operator-precedence-and-parenthesization`): an expression that
  needs parens to preserve semantics keeps them; an expression that
  does not need parens does not gain them.
- **Preludes**: HLSL, CUDA, and C++ emitted code includes the
  matching `prelude/` header or NVAPI macro (`#preludes`). GLSL,
  Metal, and WGSL emitted code does **not** include a `prelude/`
  header (the Metal "prelude" is the `metal_stdlib` system include,
  not a `prelude/` header).

### Not testable through slangc (record under `## Untested claims`)

- **`IArtifact` object layout** (`#inputs-and-outputs`). The artefact
  wrapper is an internal C++ structure; the user-observable
  consequence is "successful compile produces text", which is
  already implied by every other test in this bundle.
- **`SourceMap` companion** (`#source-writer-abstraction`). The
  source-map is a side product of the same `SourceWriter` and is
  exposed via API; not directly visible through `-target <text>`
  output. Recorded as a doc-gap-adjacent item.
- **`LineDirectiveMode` configuration** beyond the default. The
  `-line-directive-mode` switch is documented in the user guide;
  this bundle does not exercise alternative modes because that is a
  command-line surface area, not an emit-stage claim.
- **Dependency-file output `.d`** (`#dependency-file-output`). The
  `-depfile` flag emits a side artefact; this bundle stays focused
  on the per-target text emission described in the rest of the doc.
  Recorded under `## Untested claims`.
- **Torch glue** (`#backends` > Torch). Requires a host C++ compiler
  + PyTorch headers we cannot assume in the no-GPU runner.
- **LLVM / native via `slang-llvm`** (`#backends` > LLVM). Requires
  the LLVM JIT backend.
- **VM bytecode** (`#backends` > VM). Exercised by `INTERPRET`-style
  tests in lower-level bundles; this bundle is about per-target
  text emit.
- **Slang round-trip** (`#backends` > Slang round-trip). `-target
  slang` is a debugging output; behaviors here are covered by
  syntax-reference bundles.
- **Adding a new backend** (`#adding-a-new-backend`). A developer
  guide, not a user-observable behavior.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 10 to 20 `.slang` test files. The bundle is multi-backend-heavy:
   the typical file carries multiple `//TEST:SIMPLE` directives, one
   per text-emit target where the claim is observable. Use distinct
   `filecheck=<NAME>` labels (e.g. `HLSL`, `GLSL`, `SPIRV`, `METAL`,
   `WGSL`, `CUDA`, `CPP`) and per-target CHECK prefixes in the
   source comments.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/06-emit.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/llm-generated/pipeline/05-ir-passes.md`
- `docs/llm-generated/cross-cutting/targets.md`
- `docs/llm-generated/target-pipelines/hlsl.md`
- `docs/llm-generated/target-pipelines/spirv.md`
- `docs/llm-generated/target-pipelines/metal.md`
- `docs/llm-generated/target-pipelines/wgsl.md`
- `docs/llm-generated/target-pipelines/cuda.md`
- `docs/llm-generated/target-pipelines/index.md`

If you would cite anything else, stop and instead record a doc-gap
finding in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm the spelling of a per-target
emit marker (`[[buffer(N)]]`, `OpExecutionMode`, `@workgroup_size`,
…). You may **not** mine them for behavioral claims that the doc
does not make.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-c-like.{h,cpp}`
- `source/slang/slang-emit-hlsl.{h,cpp}`
- `source/slang/slang-emit-glsl.{h,cpp}`
- `source/slang/slang-emit-spirv.cpp`
- `source/slang/slang-emit-metal.{h,cpp}`
- `source/slang/slang-emit-wgsl.{h,cpp}`
- `source/slang/slang-emit-cpp.{h,cpp}`
- `source/slang/slang-emit-cuda.{h,cpp}`
- `source/slang/slang-emit-source-writer.h`
- `source/slang/slang-emit-precedence.h`

## Test directives

Per-target text emit is what this bundle is about, so the default is:

```
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda  -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp   -entry main -stage compute
```

Per-target CHECK patterns go in source comments tagged with the
matching `filecheck=<NAME>` prefix:

```
// HLSL: numthreads(8, 1, 1)
// GLSL: local_size_x = 8
// SPIRV: OpExecutionMode %main LocalSize 8 1 1
// METAL: kernel
// WGSL: @workgroup_size(8
// CUDA: __global__
// CPP: SLANG_PRELUDE_EXPORT
```

Do not use any GPU-only directive.

## Lessons captured for emit-stage tests

These are in addition to the universal lessons in `_common.md`.

- **`[[...]]` is a FileCheck variable reference, not a literal.**
  Metal's `[[kernel]]`, `[[buffer(N)]]`, `[[thread_position_in_grid]]`
  and HLSL's `[[vk::binding(...)]]` cannot be checked with a literal
  `[[kernel]]` pattern — FileCheck reports `undefined variable: kernel`.
  Match the bare token (`kernel`, `thread_position_in_grid`,
  `buffer(0)`) instead, or escape the brackets.
- **DCE strips locally-unused code before emit.** To observe an
  arithmetic result in target text, write it to an
  `RWStructuredBuffer`, return it from an entry point, or use it in a
  side-effecting printf/atomic. A pure-internal computation is
  removed before the CHECK can see it.
- **Identifier mangling varies per target.** A Slang local `a` may
  appear as `a_0` on HLSL/GLSL/Metal/WGSL, `globalParams_0.a_0` on
  CUDA via param-block lowering, or
  `(slang_bit_cast<GlobalParams_0*>(globalParams_0))->a_0` on CPP.
  Use FileCheck wildcards (`a_{{[0-9]+}}`, `{{.*}}->a_`, `{{.*}}.a_`)
  rather than literal `a_0`.
- **Resource binding markers differ per target.** The same
  `RWStructuredBuffer<int> buf` becomes `register(u0)` on HLSL,
  `layout(std430, binding = 0)` on GLSL, `Binding 0, DescriptorSet
  0` on SPIR-V, `[[buffer(N)]]` on Metal (a positional argument
  binding, not necessarily 0), `@binding(0) @group(0)` on WGSL, and
  a `GlobalParams` field on CUDA/CPP.
- **Metal does not honour HLSL `register(...)` or Vulkan
  `[[vk::binding(...)]]` for buffer-index selection.** Metal assigns
  buffer indices positionally from the entry-point parameter list.
  Do not check for a specific Metal `buffer(N)` index based on the
  HLSL annotation.
- **The prelude path on HLSL is `nvHLSLExtns.h` conditional on
  `SLANG_HLSL_ENABLE_NVAPI`.** HLSL does not unconditionally
  `#include` a prelude header; the doc still groups it with
  prelude-having targets because the header ships under `prelude/`.
  Pin a more reliable HLSL marker (`#pragma pack_matrix` or the
  conditional `#ifdef SLANG_HLSL_ENABLE_NVAPI`) rather than asserting
  an unconditional `#include`.
- **CUDA and C++ unconditionally `#include "...slang-cuda-prelude.h"`
  / `...slang-cpp-prelude.h"`.** The path is absolute and contains
  the host file system path; match a substring like
  `slang-cuda-prelude.h` rather than the full path.
- **`#line` directives appear in HLSL, GLSL, Metal, CUDA, and CPP
  emit by default.** WGSL and SPIR-V text do not show `#line`
  directives.
- **Operator-precedence assertions need two checks**: one for the
  parens-required case (FileCheck `(.*+.*) *.*`) and one for the
  parens-not-required case (FileCheck a `*` immediately followed by
  `+` with no enclosing parens). Use the same file for both to
  share the entry point.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/06-emit.md` (or one of the listed secondary docs).
- [ ] Each `.slang` file carries `//TEST:SIMPLE` directives for
      every text-emit target where the claim is observable; an
      emit-stage claim tested on a single target is a smell.
- [ ] Per-target CHECK patterns avoid raw `[[...]]` (FileCheck
      variable syntax) and use FileCheck wildcards
      (`{{[0-9]+}}`, `{{.*}}`) for mangled identifiers.
- [ ] No test depends on a GPU. SIMPLE text-emit is the only
      directive used in this bundle. (DIAGNOSTIC_TEST has no role
      here — emit-stage diagnostics belong in
      `cross-cutting/diagnostics`.)
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-emit.cpp:NNNN`", stop and re-read the doc.
- [ ] `## Doc gaps observed` records the claims the doc makes that
      were not testable through slangc text emit (Torch, LLVM, VM,
      `IArtifact`, `.d` dependency files).
