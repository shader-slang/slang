---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:00:00+00:00
source_commit: 3250005059a2746ebc504a9d3f71ed112f1f2b94
watched_paths_digest: 58e2222aab32a056bd6816440b7e7723f8b7eacefa98bb9064bbf1d76287a24d
source_doc: docs/llm-generated/pipeline/06-emit.md
source_doc_digest: a2de47410ee74b8edd850e32a2ffdc06270316e9feb047f181ef4397564602e9
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/06-emit

## Intent

Tests verify the code-emission stage described in
[`docs/llm-generated/pipeline/06-emit.md`](../../../docs/llm-generated/pipeline/06-emit.md):
that the emit dispatcher (`emitEntryPointsSourceFromIR`) routes a
compile to the right per-target backend, that each backend emits
target-shaped text (HLSL `register(uN)` / `[numthreads]`, GLSL
`#version` / `layout(...)`, SPIR-V `OpCapability` / `OpEntryPoint` /
`OpExecutionMode` / `OpDecorate ... Binding`, Metal
`#include <metal_stdlib>` / `kernel` / `buffer(N)`, WGSL `@binding /
@group / @compute / @workgroup_size`, C++ `slang-cpp-prelude.h` +
`SLANG_PRELUDE_EXPORT`, CUDA `slang-cuda-prelude.h` +
`extern "C" __global__`), that the `SourceWriter` emits `#line`
directives so downstream compilers can map errors back to the user's
source, that the precedence helper inserts the minimum number of
parentheses (keeps `(a + b) * c`'s parens, drops parens around
`a * b` in `a * b + c`), and that the preludes table holds — CUDA and
C++ unconditionally `#include` their prelude; HLSL conditionally
includes the NVAPI extension header; GLSL / Metal / WGSL do not
include a `prelude/` header at all.

This bundle is multi-backend-heavy by design: the emit stage is the
**only** stage in the compiler whose defining characteristic is
per-target text output. Most tests carry one `//TEST:SIMPLE`
directive per text-emit target.

Coverage strategy: one all-targets dispatcher smoke test
(`emit-dispatcher-all-targets.slang`), one focused single-target
test per documented backend (HLSL / GLSL / SPIR-V / Metal / WGSL /
CUDA / C++), one cross-target entry-point-marker test, two
cross-target precedence tests (paren-required, paren-omitted), one
cross-target resource-binding-shape test, one cross-target
cbuffer-shape test, one cross-target struct-emit test (shared
CLikeSourceEmitter), one cross-target vector-type-spelling test
(shared CLikeSourceEmitter), one cross-target `#line`-directive test
(SourceWriter), and one preludes-include test (across HLSL / CUDA /
C++ for positive coverage and GLSL / WGSL for negative coverage).

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                | Claim (one line)                                                                                                                                | Tests                                                                                                                                                              |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C-01     | [#emit-dispatcher](../../../docs/llm-generated/pipeline/06-emit.md#emit-dispatcher)                                                                                                   | `emitEntryPointsSourceFromIR` dispatches to the per-target backend for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++.                          | `emit-dispatcher-all-targets.slang`                                                                                                                                |
| C-02     | [#hlsl](../../../docs/llm-generated/pipeline/06-emit.md#hlsl)                                                                                                                         | The HLSL backend emits HLSL with `[numthreads(...)]` and `register(uN)`.                                                                        | `hlsl-numthreads-and-register.slang`                                                                                                                               |
| C-03     | [#glsl](../../../docs/llm-generated/pipeline/06-emit.md#glsl)                                                                                                                         | The GLSL backend emits a `#version` GLSL source with `layout(local_size_x = ...)` and `layout(std430, binding = N)`.                            | `glsl-version-and-layout.slang`                                                                                                                                    |
| C-04     | [#spir-v-direct](../../../docs/llm-generated/pipeline/06-emit.md#spir-v-direct)                                                                                                       | The direct SPIR-V backend emits SPIR-V text with `OpCapability Shader`, `OpEntryPoint GLCompute`, `OpExecutionMode LocalSize`, and `OpDecorate Binding/DescriptorSet`. | `spirv-asm-entry-point-and-decorate.slang`                                                                                                                         |
| C-05     | [#metal](../../../docs/llm-generated/pipeline/06-emit.md#metal)                                                                                                                       | The Metal backend emits Metal with `#include <metal_stdlib>`, a `kernel` function qualifier, `thread_position_in_grid` input, and `buffer(N)` arguments. | `metal-kernel-and-buffer-attr.slang`                                                                                                                               |
| C-06     | [#wgsl](../../../docs/llm-generated/pipeline/06-emit.md#wgsl)                                                                                                                         | The WGSL backend emits WGSL with `@binding(N) @group(N)` resource markers and `@compute` / `@workgroup_size` entry-point markers.               | `wgsl-binding-and-workgroup.slang`                                                                                                                                 |
| C-07     | [#c](../../../docs/llm-generated/pipeline/06-emit.md#c)                                                                                                                               | The C++ backend emits C++ that includes `slang-cpp-prelude.h` and exports a `SLANG_PRELUDE_EXPORT`-tagged entry point.                          | `cpp-prelude-and-export.slang`                                                                                                                                     |
| C-08     | [#cuda](../../../docs/llm-generated/pipeline/06-emit.md#cuda)                                                                                                                         | The CUDA backend emits CUDA that includes `slang-cuda-prelude.h` and exports `extern "C" __global__ void main(...)`.                            | `cuda-prelude-and-global.slang`                                                                                                                                    |
| C-09     | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends)                                                                                                                 | A `[shader("compute")][numthreads(N,1,1)]` entry point survives the pipeline as the target's compute-kernel marker on every text backend.       | `entry-point-markers-each-text-target.slang`                                                                                                                       |
| C-10     | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base)                                                                                             | The shared `CLikeSourceEmitter` base emits a `struct` declaration on every C-like text target; SPIR-V emits the same type as `OpTypeStruct`.    | `shared-c-like-base-struct-emit.slang`                                                                                                                             |
| C-11     | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base)                                                                                             | A `float3` parameter is emitted with each target's native vector spelling (HLSL/Metal `float3`, GLSL `vec3`, WGSL `vec3<f32>`, CUDA `float3`, CPP `Vector<float,3>`). | `vector-type-shape-per-target.slang`                                                                                                                               |
| C-12     | [#source-writer-abstraction](../../../docs/llm-generated/pipeline/06-emit.md#source-writer-abstraction)                                                                               | `SourceWriter::advanceToSourceLocation` emits `#line` directives on the HLSL, GLSL, Metal, CUDA, and C++ text targets.                          | `source-writer-emits-line-directives.slang`                                                                                                                        |
| C-13     | [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization)                                                 | The precedence helper keeps parens that are required for correctness (e.g. `(a + b) * c` keeps its outer parens on every C-like target).        | `precedence-needed-parens.slang`                                                                                                                                   |
| C-14     | [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization)                                                 | The precedence helper omits parens that are NOT required (e.g. `a * b + c` does not gain a `(a * b)` wrapping on any C-like target).            | `precedence-omits-unneeded-parens.slang`                                                                                                                           |
| C-15     | [#preludes](../../../docs/llm-generated/pipeline/06-emit.md#preludes)                                                                                                                 | CUDA and C++ emit `#include` of their matching `prelude/` header; HLSL emits the NVAPI extension macro; GLSL/Metal/WGSL emit no `prelude/` header. | `preludes-included-by-c-and-cuda.slang`                                                                                                                            |
| C-16     | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends)                                                                                                                 | A resource binding survives the pipeline in the target's native shape: HLSL `register(uN)`, GLSL `layout(std430, binding = N, set = S)`, SPIR-V `OpDecorate Binding/DescriptorSet`, Metal `buffer(N)`, WGSL `@binding(N) @group(N)`. | `resource-binding-shape-per-target.slang`                                                                                                                          |
| C-17     | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends)                                                                                                                 | A `cbuffer` is emitted in its target-shaped form: HLSL `cbuffer ... : register(b)`, GLSL `layout(std140) uniform`, SPIR-V Block-decorated struct on Uniform binding, Metal `constant *`, WGSL `var<uniform>`. | `cbuffer-shape-per-target.slang`                                                                                                                                   |

## Tests in this bundle

| File                                            | Intent     | Doc anchor                                  |
| ----------------------------------------------- | ---------- | ------------------------------------------- |
| `emit-dispatcher-all-targets.slang`             | functional | `#emit-dispatcher`                          |
| `hlsl-numthreads-and-register.slang`            | functional | `#hlsl`                                     |
| `glsl-version-and-layout.slang`                 | functional | `#glsl`                                     |
| `spirv-asm-entry-point-and-decorate.slang`      | functional | `#spir-v-direct`                            |
| `metal-kernel-and-buffer-attr.slang`            | functional | `#metal`                                    |
| `wgsl-binding-and-workgroup.slang`              | functional | `#wgsl`                                     |
| `cpp-prelude-and-export.slang`                  | functional | `#c`                                        |
| `cuda-prelude-and-global.slang`                 | functional | `#cuda`                                     |
| `entry-point-markers-each-text-target.slang`    | functional | `#backends`                                 |
| `shared-c-like-base-struct-emit.slang`          | functional | `#shared-c-like-base`                       |
| `vector-type-shape-per-target.slang`            | functional | `#shared-c-like-base`                       |
| `source-writer-emits-line-directives.slang`     | functional | `#source-writer-abstraction`                |
| `precedence-needed-parens.slang`                | functional | `#operator-precedence-and-parenthesization` |
| `precedence-omits-unneeded-parens.slang`        | functional | `#operator-precedence-and-parenthesization` |
| `preludes-included-by-c-and-cuda.slang`         | functional | `#preludes`                                 |
| `resource-binding-shape-per-target.slang`       | functional | `#backends`                                 |
| `cbuffer-shape-per-target.slang`                | functional | `#backends`                                 |

## Doc gaps observed

- The doc lists `slang-emit.cpp` line numbers ("line 893", "line 2418
  at `source_commit`") for `linkAndOptimizeIR` and
  `emitEntryPointsSourceFromIR`. These are navigation aids, not
  user-facing claims; no test anchors them.
- The doc's preludes table groups HLSL with the prelude-having
  targets, but the HLSL emit only includes `nvHLSLExtns.h`
  conditionally (`#ifdef SLANG_HLSL_ENABLE_NVAPI`). A one-line note
  in the doc that HLSL's prelude inclusion is conditional (unlike
  CUDA / C++) would clarify what a test should check; the test in
  this bundle pins the `SLANG_HLSL_ENABLE_NVAPI` macro rather than
  an unconditional `#include`.
- The doc's `## Operator precedence and parenthesization` section
  says the helper inserts "minimal" parentheses but does not give
  any example of an expression that requires parens vs. one that
  does not. Two tests in this bundle cover the two cases; an
  example pair in the doc would let an agent test these more
  precisely.
- The doc's `## Source-writer abstraction` section mentions
  `LineDirectiveMode` configures the directive style (C / GLSL /
  none) but does not state which mode is the default per target.
  The test in this bundle pins only the existence of `#line`
  directives on HLSL/GLSL/Metal/CUDA/C++; SPIR-V and WGSL do not
  emit `#line` directives by default but the doc does not say so
  explicitly.
- The doc's `## Backends` table for Metal says "Emits Metal Shading
  Language" but does not state that Metal assigns `buffer(N)`
  indices positionally from the entry-point parameter list (i.e.
  Metal does not honour `register(uN)` or `vk::binding(N)` for
  buffer index). A one-line clarification in the doc would let the
  test pin Metal's `buffer(N)` to a specific index; the test in
  this bundle accepts any integer.
- The doc's `## Backends` table lists CUDA as emitting CUDA source,
  but does not state that CUDA uses a `GlobalParams` struct +
  `SLANG_globalParams` shim to surface module-scope `uniform`
  declarations. The test in this bundle confirms the
  `SLANG_globalParams`/`__constant__ GlobalParams_0` shape, which
  the doc does not currently describe.
- The doc's `## Preludes` table column for "C++ host" points at
  `slang-cpp-host-prelude.h`, but the doc does not state when this
  prelude is selected vs. `slang-cpp-prelude.h`. We test only the
  shader-side prelude (`slang-cpp-prelude.h`) because that is the
  one chosen by `-target cpp` for a compute entry point.

## Out of scope (no-GPU runner)

- **Torch glue** (`#backends` > Torch). Requires a host C++ compiler
  + PyTorch headers we cannot assume in the no-GPU runner.
- **LLVM / native via `slang-llvm`** (`#backends` > LLVM). Requires
  the LLVM JIT.
- **VM bytecode** (`#backends` > VM). Exercised by `INTERPRET`-style
  tests in lower-level bundles; this bundle's focus is per-target
  text emit.
- **Slang round-trip** (`#backends` > Slang round-trip). `-target
  slang` is a debugging output covered by syntax-reference bundles.
- **`IArtifact` object layout** (`#inputs-and-outputs`). Internal
  C++ structure. The user-visible consequence ("a successful
  compile produces text") is implied by every other test passing.
- **Dependency-file output `.d`** (`#dependency-file-output`). The
  `-depfile` flag emits a side artefact; this bundle stays on
  per-target text emission as described in the rest of the doc.
- **`SourceMap` companion** (`#source-writer-abstraction`).
  Source-map metadata is an API surface, not a text-emit shape.
- **Alternative `LineDirectiveMode` values** beyond the default.
  The mode is a command-line surface (`-line-directive-mode`), not
  an emit-stage invariant.
- **"Adding a new backend" workflow** (`#adding-a-new-backend`). A
  developer guide, not a user-observable behavior.
- **Binary targets that need extra ecosystem tooling**: raw SPIR-V
  binary (`-target spirv` without `-asm`) needs SPIR-V validation;
  DXIL needs DXC; MSL binary needs a Metal compiler; WGSL binary
  has no such mode. The corresponding text-emit forms (`spirv-asm`,
  `hlsl`, `metal`, `wgsl`) are tested instead.
