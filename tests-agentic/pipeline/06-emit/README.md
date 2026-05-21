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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A Slang cbuffer is emitted in its target-shaped form on each text backend: HLSL `cbuffer`, GLSL `layout(std140) uniform`, Metal `constant *`, WGSL `var<uniform>`, SPIR-V Uniform/Block-decorated struct. | functional | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`cbuffer-shape-per-target.slang`](cbuffer-shape-per-target.slang) |
| Boundary: a compute kernel with an empty body still emits all required per-target entry-point markers — every backend round-trips the minimum shape (numthreads/local_size_x/@workgroup_size/OpExecutionMode LocalSize/[[kernel]]). | boundary | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`empty-kernel-body-per-target.slang`](empty-kernel-body-per-target.slang) |
| Boundary: a large numthreads value (1024, 1, 1) round-trips through emit as the target's local-workgroup-size marker — HLSL `numthreads(1024,1,1)`, GLSL `local_size_x = 1024`, SPIR-V `LocalSize 1024 1 1`, WGSL `@workgroup_size(1024,1,1)`. | boundary | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`large-numthreads-emit-per-target.slang`](large-numthreads-emit-per-target.slang) |
| Boundary: a single-buffer compute kernel with no explicit vk::binding annotation emits the default binding index per target — HLSL `register(u0)`, GLSL `binding = 0`, SPIR-V `Binding 0`, Metal `buffer(0)`, WGSL `@binding(0)`. | boundary | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`single-buffer-kernel-no-vk-binding.slang`](single-buffer-kernel-no-vk-binding.slang) |
| Boundary: the smallest documented numthreads value (1, 1, 1) is emitted as `numthreads(1,1,1)` / `local_size_x=1` / `LocalSize 1 1 1` / `@workgroup_size(1,1,1)` — no backend collapses or omits the single-thread workgroup attribute. | boundary | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`single-thread-numthreads-emit-per-target.slang`](single-thread-numthreads-emit-per-target.slang) |
| Each backend renders an RWStructuredBuffer resource binding in its target-specific form: HLSL register(uN), GLSL layout(std430, binding=N), SPIR-V OpDecorate Binding/DescriptorSet, Metal buffer(N), WGSL @binding(N) @group(N). | functional | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`resource-binding-shape-per-target.slang`](resource-binding-shape-per-target.slang) |
| Each text-emit backend in the dispatcher emits a target-distinctive entry-point marker for a `[shader("compute")]` function with `[numthreads(16, 1, 1)]`. | functional | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`entry-point-markers-each-text-target.slang`](entry-point-markers-each-text-target.slang) |
| Stress: four resource bindings get four distinct default indices in the emit — HLSL u0..u3, GLSL binding=0..3, WGSL @binding(0..3), SPIR-V Binding 0..3. | stress | [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | [`many-buffer-bindings-stress.slang`](many-buffer-bindings-stress.slang) |
| Boundary: the C++ backend's entry-point shim emits both the per-thread (`_Thread`) and per-group (`_Group`) flavours so the host runtime can drive either dispatch level. | boundary | [#c](../../../docs/llm-generated/pipeline/06-emit.md#c) | [`cpp-host-shim-thread-and-group-entry.slang`](cpp-host-shim-thread-and-group-entry.slang) |
| The C++ backend emits C++ source that includes slang-cpp-prelude.h and exports the entry point with SLANG_PRELUDE_EXPORT. | functional | [#c](../../../docs/llm-generated/pipeline/06-emit.md#c) | [`cpp-prelude-and-export.slang`](cpp-prelude-and-export.slang) |
| Boundary: a kernel with multiple module-scope uniform globals is emitted on CUDA via the GlobalParams struct + SLANG_globalParams shim (the `__constant__ GlobalParams_N SLANG_globalParams` pattern documented in the bundle's doc-gap note). | boundary | [#cuda](../../../docs/llm-generated/pipeline/06-emit.md#cuda) | [`cuda-global-params-shim-shape.slang`](cuda-global-params-shim-shape.slang) |
| The CUDA backend emits CUDA source that includes slang-cuda-prelude.h and exports the entry point as an extern "C" __global__ function. | functional | [#cuda](../../../docs/llm-generated/pipeline/06-emit.md#cuda) | [`cuda-prelude-and-global.slang`](cuda-prelude-and-global.slang) |
| Boundary: when the same source has two compute entry points, compiling with `-entry foo` produces foo's body (and not bar's), and `-entry bar` produces bar's body (and not foo's). The dispatcher routes a single entry per compile. | boundary | [#emit-dispatcher](../../../docs/llm-generated/pipeline/06-emit.md#emit-dispatcher) | [`multi-entry-point-foo-vs-bar-emit.slang`](multi-entry-point-foo-vs-bar-emit.slang) |
| emitEntryPointsSourceFromIR dispatches to the per-target backend for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++. | functional | [#emit-dispatcher](../../../docs/llm-generated/pipeline/06-emit.md#emit-dispatcher) | [`emit-dispatcher-all-targets.slang`](emit-dispatcher-all-targets.slang) |
| The GLSL backend emits a GLSL source with #version, layout(local_size_x = ...) for the compute stage, and layout(std430, binding = N) for a buffer resource. | functional | [#glsl](../../../docs/llm-generated/pipeline/06-emit.md#glsl) | [`glsl-version-and-layout.slang`](glsl-version-and-layout.slang) |
| Stress: HLSL `[numthreads(...)]` and `register(uN)` shapes survive a kernel with multiple resources of mixed read/write classification (one RW buffer, one read-only buffer, one cbuffer). | stress | [#hlsl](../../../docs/llm-generated/pipeline/06-emit.md#hlsl) | [`hlsl-numthreads-survives-multi-resource.slang`](hlsl-numthreads-survives-multi-resource.slang) |
| The HLSL backend emits HLSL source text with [numthreads] and register(uN) annotations for resources. | functional | [#hlsl](../../../docs/llm-generated/pipeline/06-emit.md#hlsl) | [`hlsl-numthreads-and-register.slang`](hlsl-numthreads-and-register.slang) |
| The Metal backend emits Metal source with metal_stdlib include, the kernel function-qualifier, a thread_position_in_grid input, and buffer(N) markers for buffer arguments. | functional | [#metal](../../../docs/llm-generated/pipeline/06-emit.md#metal) | [`metal-kernel-and-buffer-attr.slang`](metal-kernel-and-buffer-attr.slang) |
| Stress: a deeply nested arithmetic expression `((a+b)*(c+d)) - (a*b) + c*d` is emitted with the minimum parentheses set — additions stay parenthesised when they sit under a multiplication, but mul-precedence subexpressions are not over-wrapped. | stress | [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization) | [`precedence-deeply-nested-arith-stress.slang`](precedence-deeply-nested-arith-stress.slang) |
| The precedence helper emits the minimum parentheses needed. `a * b + c` keeps neither operator wrapped (no `(a * b) + c` and no `a * (b + c)`). | functional | [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization) | [`precedence-omits-unneeded-parens.slang`](precedence-omits-unneeded-parens.slang) |
| The precedence helper keeps parentheses that are required to preserve semantics: an addition inside a multiplication context emits as `(a + b) * c` on every C-like text target. | functional | [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization) | [`precedence-needed-parens.slang`](precedence-needed-parens.slang) |
| Negative: Metal emit does NOT include a slang-*-prelude.h header. It includes the platform's metal_stdlib system header but no Slang `prelude/` header. | negative | [#preludes](../../../docs/llm-generated/pipeline/06-emit.md#preludes) | [`metal-omits-slang-prelude-include.slang`](metal-omits-slang-prelude-include.slang) |
| Negative: SPIR-V emit does NOT include any slang-*-prelude.h header (SPIR-V is not in the preludes table in the source doc). | negative | [#preludes](../../../docs/llm-generated/pipeline/06-emit.md#preludes) | [`spirv-omits-slang-prelude-include.slang`](spirv-omits-slang-prelude-include.slang) |
| Targets that ship a `prelude/` header (HLSL, CUDA, C++) emit text that references it. GLSL, Metal, and WGSL do not include a `prelude/` header (Metal pulls in metal_stdlib, which is the platform's system header, not the Slang `prelude/`). | functional | [#preludes](../../../docs/llm-generated/pipeline/06-emit.md#preludes) | [`preludes-included-by-c-and-cuda.slang`](preludes-included-by-c-and-cuda.slang) |
| A Slang `float3` parameter is emitted with the target-native vector spelling: HLSL/Metal `float3`, GLSL `vec3`, WGSL `vec3<f32>`, CUDA `float3 `, CPP `Vector<float, 3>`. | functional | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`vector-type-shape-per-target.slang`](vector-type-shape-per-target.slang) |
| Boundary: +infinity (1.0/0.0) is emitted as each target's native infinity spelling — CUDA uses SLANG_INFINITY, WGSL uses _slang_getInfinity, HLSL/GLSL/Metal/CPP emit the literal division. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`float-positive-infinity-emit-per-target.slang`](float-positive-infinity-emit-per-target.slang) |
| Boundary: NaN (0.0/0.0) is emitted as each target's native NaN-producing form — WGSL uses _slang_getNan; HLSL/GLSL/Metal/CUDA/CPP emit the literal division. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`float-nan-emit-per-target.slang`](float-nan-emit-per-target.slang) |
| Boundary: a Slang struct with three heterogeneous fields (int, float, uint) is emitted as a target-native struct declaration on every C-like text target and as an OpTypeStruct on SPIR-V. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`struct-multi-field-emit-per-target.slang`](struct-multi-field-emit-per-target.slang) |
| Boundary: a default-layout float4x4 in a cbuffer is emitted with a target-specific matrix marker — HLSL `#pragma pack_matrix(column_major)`, GLSL `layout(row_major)`, SPIR-V `RowMajor` member decoration, Metal/WGSL `_MatrixStorage_float4x4_ColMajor...`. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`matrix-default-layout-emit-per-target.slang`](matrix-default-layout-emit-per-target.slang) |
| Boundary: an explicit `row_major float4x4` in a cbuffer changes the per-target emit marker — GLSL drops the `_ColMajor` wrapper-struct suffix; SPIR-V emits `ColMajor` member decoration (because SPIR-V matrix layouts are transposed relative to source); WGSL drops the `_ColMajor` infix. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`matrix-row-major-emit-per-target.slang`](matrix-row-major-emit-per-target.slang) |
| Boundary: the float literal 0.0 is emitted as the target's native zero-literal spelling on every C-like text target and as an OpConstant on SPIR-V. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`float-literal-zero-emit-per-target.slang`](float-literal-zero-emit-per-target.slang) |
| Boundary: the int32 minimum value (-2147483648) round-trips through emit as a negative literal on each target — HLSL/Metal/CUDA wrap it as `int(-2147483648)`; GLSL emits the bare literal; WGSL casts to i32; SPIR-V encodes it as an OpConstant on %int. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`int-literal-min-emit-per-target.slang`](int-literal-min-emit-per-target.slang) |
| Boundary: the uint32 maximum value (4294967295u) round-trips through emit as the target's native unsigned literal — HLSL/GLSL/Metal/CUDA/CPP append a `U` suffix; WGSL wraps it as `u32(...)`; SPIR-V encodes it as an OpConstant on %uint. | boundary | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`uint-literal-max-emit-per-target.slang`](uint-literal-max-emit-per-target.slang) |
| The shared CLikeSourceEmitter base produces a `struct` declaration on every C-like text target (HLSL, GLSL, Metal, CUDA, C++) for a user struct. SPIR-V emits the same type as `OpTypeStruct`. WGSL emits a `struct` block. | functional | [#shared-c-like-base](../../../docs/llm-generated/pipeline/06-emit.md#shared-c-like-base) | [`shared-c-like-base-struct-emit.slang`](shared-c-like-base-struct-emit.slang) |
| Boundary/negative: SPIR-V emit does NOT produce a C `#line` directive — instead it emits `OpSource Slang 1` as the source-location anchor in the SPIR-V debug info section. | negative | [#source-writer-abstraction](../../../docs/llm-generated/pipeline/06-emit.md#source-writer-abstraction) | [`spirv-emits-opsource-not-line-directive.slang`](spirv-emits-opsource-not-line-directive.slang) |
| Boundary/negative: WGSL emit does NOT produce `#line` directives — WGSL has no `#line` syntax, so the SourceWriter on this target omits source-location anchors entirely. | negative | [#source-writer-abstraction](../../../docs/llm-generated/pipeline/06-emit.md#source-writer-abstraction) | [`wgsl-omits-line-directive.slang`](wgsl-omits-line-directive.slang) |
| SourceWriter::advanceToSourceLocation emits #line directives so downstream compilers can report errors at the user's source position. The directives appear in the HLSL, GLSL, Metal, CUDA, and C++ emit. | functional | [#source-writer-abstraction](../../../docs/llm-generated/pipeline/06-emit.md#source-writer-abstraction) | [`source-writer-emits-line-directives.slang`](source-writer-emits-line-directives.slang) |
| The direct SPIR-V backend emits SPIR-V text with OpCapability Shader, OpEntryPoint GLCompute, OpExecutionMode LocalSize, and OpDecorate Binding/DescriptorSet for a buffer resource. | functional | [#spir-v-direct](../../../docs/llm-generated/pipeline/06-emit.md#spir-v-direct) | [`spirv-asm-entry-point-and-decorate.slang`](spirv-asm-entry-point-and-decorate.slang) |
| The WGSL backend emits WGSL source with @binding(N) @group(N) for resources and @compute / @workgroup_size for the compute entry point. | functional | [#wgsl](../../../docs/llm-generated/pipeline/06-emit.md#wgsl) | [`wgsl-binding-and-workgroup.slang`](wgsl-binding-and-workgroup.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#line-893](../../../docs/llm-generated/pipeline/06-emit.md#line-893) | undocumented-behavior | The doc lists `slang-emit.cpp` line numbers ("line 893", "line 2418 at `source_commit`") for `linkAndOptimizeIR` and `emitEntryPointsSourceFromIR`. These are navigation aids, not user-facing claims; no test anchors them. |  |
| [#ifdef](../../../docs/llm-generated/pipeline/06-emit.md#ifdef) | undocumented-behavior | The doc's preludes table groups HLSL with the prelude-having targets, but the HLSL emit only includes `nvHLSLExtns.h` conditionally (`#ifdef SLANG_HLSL_ENABLE_NVAPI`). | A one-line note in the doc that HLSL's prelude inclusion is conditional (unlike CUDA / C++) would clarify what a test should check; the test in this bundle pins the `SLANG_HLSL_ENABLE_NVAPI` macro rather than an unconditional `#include`. |
| [#operator-precedence-and-parenthesization](../../../docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization) | undocumented-behavior | The doc's `## Operator precedence and parenthesization` section says the helper inserts "minimal" parentheses but does not give any example of an expression that requires parens vs. one that does not. Two tests in this bundle cover the two cases; an example pair in the doc would let an agent test these more precisely. |  |
| [#line](../../../docs/llm-generated/pipeline/06-emit.md#line) | undocumented-behavior | The doc's `## Source-writer abstraction` section mentions `LineDirectiveMode` configures the directive style (C / GLSL / none) but does not state which mode is the default per target. The test in this bundle pins only the existence of `#line` directives on HLSL/GLSL/Metal/CUDA/C++; SPIR-V and WGSL do not emit `#line` directives by default but the doc does not say so explicitly. |  |
| [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | undocumented-behavior | The doc's `## Backends` table for Metal says "Emits Metal Shading Language" but does not state that Metal assigns `buffer(N)` indices positionally from the entry-point parameter list (i.e. Metal does not honour `register(uN)` or `vk::binding(N)` for buffer index). | A one-line clarification in the doc would let the test pin Metal's `buffer(N)` to a specific index; the test in this bundle accepts any integer. |
| [#backends](../../../docs/llm-generated/pipeline/06-emit.md#backends) | undocumented-behavior | The doc's `## Backends` table lists CUDA as emitting CUDA source, but does not state that CUDA uses a `GlobalParams` struct + `SLANG_globalParams` shim to surface module-scope `uniform` declarations. The test in this bundle confirms the `SLANG_globalParams`/`__constant__ GlobalParams_0` shape, which the doc does not currently describe. |  |
| [#preludes](../../../docs/llm-generated/pipeline/06-emit.md#preludes) | undocumented-behavior | The doc's `## Preludes` table column for "C++ host" points at `slang-cpp-host-prelude.h`, but the doc does not state when this prelude is selected vs. `slang-cpp-prelude.h`. We test only the shader-side prelude (`slang-cpp-prelude.h`) because that is the one chosen by `-target cpp` for a compute entry point. |  |

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
