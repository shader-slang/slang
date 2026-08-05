---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-04T00:00:00+00:00
source_commit: 7e725f15572c6589ee6d738a8856fb3348f11617
watched_paths_digest: e3702b2bb4b3b93680f0aa20c0d2faddbe420daf72e125260bb8e1dca2064879
source_doc: docs/generated/design/cross-cutting/core-module.md
source_doc_digest: 01ecdc2cb8dd909148390b350a26194c62dcf5b532f38a1eb614359783989fea
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/cross-cutting/core-module

## Intent

This bundle exercises
[`docs/generated/design/cross-cutting/core-module.md`](../../../../design/cross-cutting/core-module.md)
— the doc that describes what ships with the compiler: the embedded
core module and its HLSL / diff meta-modules, the embedded GLSL
module, the separately compiled standard modules loaded on demand by a
qualified `import`, and the per-target preludes prepended to textual
output.

The coverage strategy follows the doc's own split between what user
code can _see_ and what a target _emits_:

- **Module identity and visibility** claims (the core module is one
  module named `core`; its aliases, `Optional` / `Tuple`,
  `IRangedValue`, `IDifferentiable` / `DifferentialPair` and the diff
  attribute vocabulary are in scope with no import; standard modules
  are **not**) are target-independent, so they use `//TEST:INTERPRET`
  or a single emission directive, plus `//DIAGNOSTIC_TEST` for the
  three negative cases.
- **Per-target lowering** claims (the HLSL meta-module's `dot` /
  `length` / `mul`, its buffer / texture / sampler types, the core
  vector-matrix vocabulary, the GLSL module's aliases) fan out one
  `//TEST:SIMPLE(filecheck=<PREFIX>)` directive per text-emit target —
  `hlsl`, `glsl`, `spirv-asm`, `metal`, `wgsl`, `cuda`, `cpp` — so a
  regression in any single back end fails on its own line.
- **Boundary probes** instantiate the mandatory integer and float axes
  on the base types the doc names (`int8_t`, `int32_t`, `uint32_t`,
  `float32_t`): MIN, MAX, MAX+1 wrap, `±inf`, NaN and signed zero, one
  boundary value per file.

Per the bundle prompt, no test here exercises an atomic intrinsic.

## Functional coverage

| Claim                                                                                                                                                                                                                                                                               | Intent     | Anchor                                                                                                          | Tests                                                                                                |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | --------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| StructuredBuffer and RWStructuredBuffer from the HLSL meta-module legalize to each target's own binding shape, and the read-only one keeps a read-only marker where the target has one.                                                                                             | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`hlsl-structuredbuffer-per-target.slang`](hlsl-structuredbuffer-per-target.slang)                   |
| Texture2D, RWTexture2D and SamplerState from the HLSL meta-module lower to each target's own texture and sampler vocabulary.                                                                                                                                                        | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`hlsl-texture2d-sampler-per-target.slang`](hlsl-texture2d-sampler-per-target.slang)                 |
| The core module is one identified Slang module named `core`, so `import core;` resolves to the already-loaded builtin module and core declarations stay usable.                                                                                                                     | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-module-named-core-import.slang`](core-module-named-core-import.slang)                         |
| The diff meta-module supplies the derivative attribute syntax (\[ForwardDerivative\], \[BackwardDerivative\]), the NullDifferential type, and IDifferentiable conformances for Array, Optional and Tuple.                                                                           | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`diff-module-attributes-and-conformances.slang`](diff-module-attributes-and-conformances.slang)     |
| The HLSL meta-module intrinsic dot is emitted as the target's own dot: HLSL/GLSL/Metal/WGSL dot, SPIR-V OpDot, and a generated dot\_ helper on CUDA and C++.                                                                                                                        | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`hlsl-intrinsic-dot-per-target.slang`](hlsl-intrinsic-dot-per-target.slang)                         |
| The HLSL meta-module intrinsic length is emitted as the target's own length: HLSL/GLSL/Metal/WGSL length, the SPIR-V GLSL.std.450 Length extended instruction, and a generated helper on CUDA and C++.                                                                              | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`hlsl-intrinsic-length-per-target.slang`](hlsl-intrinsic-length-per-target.slang)                   |
| The HLSL meta-module intrinsic mul keeps its name on HLSL, becomes the \* operator on GLSL/Metal/WGSL, becomes OpVectorTimesMatrix on SPIR-V and a generated helper on CUDA and C++.                                                                                                | functional | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`hlsl-intrinsic-mul-per-target.slang`](hlsl-intrinsic-mul-per-target.slang)                         |
| float32_t distinguishes -0.0 from +0.0 in its bit pattern while still comparing them equal.                                                                                                                                                                                         | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-float32-negative-zero-boundary.slang`](core-float32-negative-zero-boundary.slang)             |
| float32_t represents NaN, which compares unequal to itself.                                                                                                                                                                                                                         | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-float32-nan-boundary.slang`](core-float32-nan-boundary.slang)                                 |
| float32_t represents positive and negative infinity as the result of dividing a non-zero value by zero.                                                                                                                                                                             | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-float32-infinity-boundary.slang`](core-float32-infinity-boundary.slang)                       |
| int32_t holds its documented MIN and MAX end points, -2147483648 and 2147483647.                                                                                                                                                                                                    | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-int32-min-max-boundary.slang`](core-int32-min-max-boundary.slang)                             |
| int8_t keeps its documented MIN value of -128 when it is returned from a function.                                                                                                                                                                                                  | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-int8-min-return-boundary.slang`](core-int8-min-return-boundary.slang)                         |
| uint32_t addition wraps at the MAX boundary: uint32_t(0xFFFFFFFF) + 1 is 0.                                                                                                                                                                                                         | boundary   | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | [`core-uint32-max-plus-one-wraps.slang`](core-uint32-max-plus-one-wraps.slang)                       |
| An explicit import glsl retrieves the GLSL builtin module, after which its aliases resolve and lower to the target's own spelling.                                                                                                                                                  | functional | [#glsl-module](../../../../design/cross-cutting/core-module.md#glsl-module)                                     | [`glsl-module-import-glsl-name.slang`](glsl-module-import-glsl-name.slang)                           |
| The GLSL builtin module is opt-in: without the GLSL module enabled, vec3 is an undefined identifier.                                                                                                                                                                                | negative   | [#glsl-module](../../../../design/cross-cutting/core-module.md#glsl-module)                                     | [`glsl-module-not-loaded-by-default.slang`](glsl-module-not-loaded-by-default.slang)                 |
| The GLSL module's vec3 and mat4 aliases name the same types as float3 and float4x4, so a shader written with GLSL-flavoured names emits each target's native vector and matrix spelling.                                                                                            | functional | [#glsl-module](../../../../design/cross-cutting/core-module.md#glsl-module)                                     | [`glsl-module-aliases-per-target.slang`](glsl-module-aliases-per-target.slang)                       |
| GLSL, Metal, WGSL and SPIR-V output carries no prelude/ header reference; their builtin vocabulary comes from the back end or the downstream toolchain.                                                                                                                             | functional | [#preludes](../../../../design/cross-cutting/core-module.md#preludes)                                           | [`prelude-absent-from-glsl-metal-wgsl-spirv.slang`](prelude-absent-from-glsl-metal-wgsl-spirv.slang) |
| The C++ prelude is referenced from -target cpp output: the emitted text pulls in slang-cpp-prelude.h and uses its SLANG_PRELUDE_EXPORT macro on the entry point.                                                                                                                    | functional | [#preludes](../../../../design/cross-cutting/core-module.md#preludes)                                           | [`prelude-cpp-referenced-from-emit.slang`](prelude-cpp-referenced-from-emit.slang)                   |
| The CUDA prelude is referenced from -target cuda output: the emitted text pulls in slang-cuda-prelude.h and declares the entry point with extern "C" \_\_global\_\_.                                                                                                                | functional | [#preludes](../../../../design/cross-cutting/core-module.md#preludes)                                           | [`prelude-cuda-referenced-from-emit.slang`](prelude-cuda-referenced-from-emit.slang)                 |
| Standard-module declarations are not implicitly in scope: the workgraph module's DispatchNodeInputRecord is an undefined identifier without import experimental.workgraph.                                                                                                          | negative   | [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | [`standard-module-workgraph-not-implicit.slang`](standard-module-workgraph-not-implicit.slang)       |
| The neural standard module is declared \[ExperimentalModule\], so import slang.neural is refused unless experimental features are enabled.                                                                                                                                          | negative   | [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | [`standard-module-neural-experimental-gate.slang`](standard-module-neural-experimental-gate.slang)   |
| import experimental.workgraph resolves the separately compiled workgraph standard module and brings its record types and Barrier flag enums into scope.                                                                                                                             | functional | [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | [`standard-module-workgraph-import-resolves.slang`](standard-module-workgraph-import-resolves.slang) |
| Each core-module scalar alias names the same builtin type it aliases: int32_t is int, uint32_t is uint, float32_t is float, float64_t is double and float16_t is half.                                                                                                              | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-scalar-aliases-are-builtin-types.slang`](core-scalar-aliases-are-builtin-types.slang)         |
| IDifferentiable and the DifferentialPair magic type are declared in the core module itself, so a differentiable function and fwd_diff work without importing the diff meta-module.                                                                                                  | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-differentialpair-declared-in-core.slang`](core-differentialpair-declared-in-core.slang)       |
| Optional&lt;T&gt; and Tuple&lt;T...&gt; are core-module types, usable from user code with no import.                                                                                                                                                                                | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-optional-and-tuple-without-import.slang`](core-optional-and-tuple-without-import.slang)       |
| The core module's \_\_implicit_conversion declarations admit int-to-float argument conversion while an exact-match overload still wins over a converting one.                                                                                                                       | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-implicit-conversion-admitted.slang`](core-implicit-conversion-admitted.slang)                 |
| The core module's vector and matrix types are emitted in each target's own spelling: float3/float4x4 on HLSL, vec3/mat3x3 on GLSL, vec3&lt;f32&gt;/mat3x3&lt;f32&gt; on WGSL, float3/matrix&lt;float,..&gt; on Metal, float3/makeMatrix on CUDA and Vector/Matrix templates on C++. | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-vector-matrix-types-per-target.slang`](core-vector-matrix-types-per-target.slang)             |
| The globallycoherent modifier declared by the core module's syntax declarations reaches emit as HLSL globallycoherent, GLSL coherent and the SPIR-V Coherent decoration.                                                                                                            | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-globallycoherent-syntax-modifier.slang`](core-globallycoherent-syntax-modifier.slang)         |
| The IRangedValue interface and its per-scalar extensions let a generic constrained on IRangedValue read minValue and maxValue for int32_t, uint32_t and float32_t.                                                                                                                  | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-irangedvalue-generic-extensions.slang`](core-irangedvalue-generic-extensions.slang)           |
| The pointer-width aliases size_t, usize_t and ssize_t are in scope without any import and hold integral values.                                                                                                                                                                     | functional | [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | [`core-size-aliases-resolve.slang`](core-size-aliases-resolve.slang)                                 |

## Untested claims

| Claim                                                                                                                                                                                                                                                      | Reason               | Anchor                                                                                                          | Why untested                                                                                                                                                                                                                                |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------- | --------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The HLSL meta-module gives declarations per-target spellings with `__target_intrinsic(<target>, <text>)`, the example given being the `hlsl` and `cuda` spellings of `RayDesc` and its fields.                                                             | gpu-dxr              | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | `RayDesc` is only reachable from a ray-tracing entry point, which this runner cannot compile or run. A `closesthit` shader emitted for HLSL and for CUDA would show the two spellings side by side.                                         |
| The diff meta-module supplies the PyTorch-facing `TensorView`, `DiffTensorView` and `TorchTensor` types.                                                                                                                                                   | gpu-cuda             | [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | These types only lower on the CUDA / torch emit path, which needs the nvrtc + PyTorch toolchain the runner lacks. A `-target torch` emission test would pin their generated binding code.                                                   |
| The full intrinsic list — including every atomic operation's per-target spelling — is deliberately not enumerated by this doc, which defers to the `*.meta.slang` files.                                                                                   | internal-source-fact | [#what-is-not-in-this-document](../../../../design/cross-cutting/core-module.md#what-is-not-in-this-document)   | There is no doc claim about an individual intrinsic's per-target spelling to anchor to, and mining the meta files for behaviour the doc does not state is out of contract, so this bundle writes no atomic-intrinsic test.                  |
| The neural standard module `__include`s a specific set of sources — vector and storage abstractions, matrix-multiply backends, layer / activation / encoder interfaces and support code — and its `unit-test/` sources compile only under a develop build. | internal-source-fact | [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | The claim is about which files compose the module rather than about a name user code can observe; the doc names no public symbol whose presence would witness a particular `__include`.                                                     |
| The build substitutes `SLANG_STANDARD_MODULE_DIR_NAME` and the per-module file-name variables into `slang-standard-module-config.h.in`, and `findStandardModulePath` appends the qualified name plus `.slang-module`.                                      | internal-source-fact | [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | The generated header is an internal build product; its only user-visible consequence — that `slang.neural` and `experimental.workgraph` resolve at all — is already covered by the two standard-module tests.                               |
| `SLANG_EMBED_CORE_MODULE=ON` bakes the precompiled core module into `libslang`, while `OFF` keeps the C++ build independent and surfaces meta-source errors from the separate `generate_core_module` step.                                                 | compile-time-toggle  | [#building-the-core-module](../../../../design/cross-cutting/core-module.md#building-the-core-module)           | The option is baked into the binary under test, so one build can only ever observe one side of it, and nothing in a compiled shader distinguishes the two.                                                                                  |
| One `slang-bootstrap -compile-core-module` run produces `slang-core-module-without-timestamp.bin`, the embeddable core-module header and the embeddable GLSL-module header.                                                                                | needs-cli-test       | [#building-the-core-module](../../../../design/cross-cutting/core-module.md#building-the-core-module)           | These are build products of a bootstrap invocation, not of a shader compile. A wrapper script running `slang-bootstrap` with `-save-core-module` / `-save-core-module-bin-source` and inspecting the three outputs is what would verify it. |
| The runtime cache file is `[uint64_t library timestamp][serialized module bytes]`, a zero timestamp is a failure sentinel rejected by `write`, and a packaging step that rewrites timestamps makes the runtime regenerate the cache.                       | needs-unit-test      | [#the-runtime-core-module-cache](../../../../design/cross-cutting/core-module.md#the-runtime-core-module-cache) | The format is implemented by `BuiltinModuleCache::read` / `write` in C++ with no slangc CLI surface. A unit test writing a cache with a zero and with a mismatched timestamp would verify both the sentinel and the invalidation.           |
| The GLSL builtin module is loaded at global-session creation when `SlangGlobalSessionDesc::enableGLSL` is set.                                                                                                                                             | needs-unit-test      | [#glsl-module](../../../../design/cross-cutting/core-module.md#glsl-module)                                     | `enableGLSL` is a field on the session-creation descriptor. A C++ unit test creating one session with the flag and one without would show the load happens at creation rather than at the first `import glsl`.                              |
| The steps for adding a new built-in: choose the home module, declare it with `__intrinsic_op` / `__target_intrinsic`, add a prelude entry if it needs a runtime helper, rebuild through `generate_core_module_headers`, and add tests.                     | process-doc          | [#adding-a-new-built-in](../../../../design/cross-cutting/core-module.md#adding-a-new-built-in)                 | A contributor walkthrough rather than a compiler behaviour; each step's observable outcome is already covered by the intrinsic-lowering and prelude tests in this bundle.                                                                   |

## Doc gaps observed

| Anchor                                                                                                          | Kind                  | Gap                                                                                                                                                                                                                                                                                                                  | Suggested addition                                                                                                                                                                                                                          |
| --------------------------------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#core-module](../../../../design/cross-cutting/core-module.md#core-module)                                     | missing-example       | The section names `mul`, `dot` and `length` as HLSL-compatibility intrinsics but never shows what any of them becomes on a non-HLSL target, so a reader cannot tell that `mul` stops being a call at all on GLSL / Metal / WGSL while `dot` and `length` keep their names there.                                     | Add a "one intrinsic, several spellings" table under this section giving the emitted form of `mul` for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA and C++, with a two-line source snippet above it.                                              |
| [#glsl-module](../../../../design/cross-cutting/core-module.md#glsl-module)                                     | undocumented-behavior | The section says the GLSL builtin module is loaded "at creation time when `SlangGlobalSessionDesc::enableGLSL` is set" and that `import glsl` then retrieves it, but it does not say what happens when a compile that never set `enableGLSL` writes `import glsl;` — which `slangc` accepts.                         | Add a sentence stating whether `import glsl` loads the module on demand when `enableGLSL` was not set, and name the `slangc` flag (`-allow-glsl`) that makes the GLSL names available without an explicit import.                           |
| [#preludes](../../../../design/cross-cutting/core-module.md#preludes)                                           | missing-surface       | The prelude table lists header files and their target, but a reader cannot tell what a prelude looks like in the output. These tests had to discover empirically that the C++ and CUDA emit contain an `#include` of the header path plus markers such as `SLANG_PRELUDE_EXPORT` that the header defines.            | Add a three-line excerpt of a `-target cpp` emit showing the `#include "slang-cpp-prelude.h"` line and one macro the header defines, so the "prepended to textual target output" claim has a concrete shape.                                |
| [#standard-modules](../../../../design/cross-cutting/core-module.md#standard-modules)                           | undocumented-behavior | The section records that both modules declare `[ExperimentalModule]` but never says what that attribute does to a consumer. In practice `import slang.neural` and `import experimental.workgraph` are both refused with E00104 until experimental features are enabled.                                              | Add one sentence after the module table stating that `[ExperimentalModule]` makes the import fail with E00104 unless the compile enables experimental features, and name the `slangc` flag (`-experimental-feature`).                       |
| [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | missing-surface       | The section says the core module "declares modifier syntax (`constexpr`, `globallycoherent`, `pervertex`, ...) via `syntax` declarations" but does not say which of those modifiers survive to emitted code and in what form; of the three named, only `globallycoherent` was found to have a target-visible marker. | Add a note to the modifier list marking which modifiers are target-visible (e.g. `globallycoherent` → HLSL `globallycoherent`, GLSL `coherent`, SPIR-V `Coherent`) and which are front-end-only.                                            |
| [#what-the-core-module-provides](../../../../design/cross-cutting/core-module.md#what-the-core-module-provides) | missing-surface       | The section lists `Optional`, `Tuple` and the `IRangedValue` interface among the core module's declarations but gives no API sketch, so a reader cannot tell how to observe any of them; the members these tests use (`hasValue` / `value` / `none`, `_0`, `minValue` / `maxValue`) were found by inspection.        | Add a one-line member list per type — `Optional<T>`: `hasValue`, `value`, the `none` literal; `Tuple<T...>`: positional `_0`, `_1`; `IRangedValue`: `static const This minValue` / `maxValue` — or link to the user-guide section for each. |

## Claim index

Numbered enumeration of the claims extracted from the source doc,
grouped by its headings. Every entry appears in exactly one of the two
tables above.

**`#what-ships-with-the-compiler`**

1. Three families ship: the embedded core module, the embedded GLSL
   module, and separately compiled standard modules loaded on demand by
   an `import` whose qualified name maps to a subdirectory (covered
   collectively by the module-identity, GLSL-module and standard-module
   rows).
2. Per-target preludes are C / C++ / CUDA headers shipped alongside
   emitted text targets and are not Slang source.

**`#core-module`**

3. `core.meta.slang` declares the base types (`int8_t`, `int32_t`,
   `int64_t`, `float`, `half`, `double`, pointer / size types) and type
   aliases.
4. The core module is a Slang module declared with the `core` name via
   `public module core;`.
5. `hlsl.meta.slang` supplies the HLSL-compatibility names —
   `Texture2D`, `RWTexture2D`, `StructuredBuffer` — and the intrinsics
   `mul`, `dot`, `length`.
6. `diff.meta.slang` supplies the derivative `attribute_syntax`
   declarations, the `NullDifferential` type, and `IDifferentiable` /
   `IDifferentiablePtrType` conformances for `Array`, `Optional` and
   `Tuple`.
7. `diff.meta.slang` also supplies the PyTorch-facing `TensorView`,
   `DiffTensorView` and `TorchTensor` types. _(untested: `gpu-cuda`)_
8. `__target_intrinsic(<target>, <text>)` gives a declaration a
   per-target spelling, e.g. the `hlsl` and `cuda` spellings of
   `RayDesc`. _(untested: `gpu-dxr`)_

**`#what-the-core-module-provides`**

9. The file opens with `public module core;` and a block of scalar
   aliases (`float16_t`, `float32_t`, `float64_t`, `int32_t`,
   `uint32_t`, `size_t`, `usize_t`, `ssize_t`).
10. Modifier syntax (`constexpr`, `globallycoherent`, `pervertex`, ...)
    is declared through `syntax` declarations.
11. The declarations cover scalar / vector / matrix types.
12. Operator overloads are mapped onto IR opcodes with `__intrinsic_op`
    and implicit-conversion costs are declared with
    `__implicit_conversion`.
13. The `IRangedValue` interface and its per-scalar extensions are
    declared here.
14. `Optional` and `Tuple` are declared here.
15. `IDifferentiable` and the `DifferentialPair` magic type are declared
    in the core module, not in the diff meta-module.
16. The HLSL meta-module layers in HLSL-named texture / sampler / buffer
    types and their intrinsics so HLSL code compiles unchanged.

**`#glsl-module`**

17. `glsl.meta.slang` provides GLSL-flavoured aliases (`vec3`, `mat4`,
    `gl_*` system values).
18. The GLSL builtin module is loaded at global-session creation when
    `SlangGlobalSessionDesc::enableGLSL` is set. _(untested:
    `needs-unit-test`)_
19. A later `import glsl` retrieves that already-loaded builtin module.

**`#standard-modules`**

20. `slang.neural` resolves to `slang/neural.slang-module` and
    `experimental.workgraph` to `experimental/workgraph.slang-module`.
21. The neural module declares `[ExperimentalModule] module neural;` and
    pulls in the rest of its directory with `__include`. _(composition
    untested: `internal-source-fact`)_
22. The experimental module declares
    `[ExperimentalModule] module workgraph;` and holds the node
    attributes, the input / output record types, and the
    `BarrierMemoryTypeFlags` / `BarrierSemanticFlags` enums.
23. The build substitutes the module directory and file-name variables
    into `slang-standard-module-config.h.in`, and
    `findStandardModulePath` appends the qualified name plus
    `.slang-module`. _(untested: `internal-source-fact`)_

**`#preludes`**

24. Prelude text is prepended to textual target output and is
    output-side: it does not participate in front-end checking.
25. `slang-cpp-prelude.h` serves C++ shader output and
    `slang-cuda-prelude.h` serves CUDA.
26. GLSL, Metal, WGSL and SPIR-V do not use a `prelude/` header in the
    same way.

**`#building-the-core-module`, `#the-runtime-core-module-cache`, `#adding-a-new-built-in`, `#what-is-not-in-this-document`**

27. `SLANG_EMBED_CORE_MODULE` ON vs OFF changes where meta-source errors
    surface. _(untested: `compile-time-toggle`)_
28. One `slang-bootstrap` run produces the standalone archive and the two
    embeddable headers. _(untested: `needs-cli-test`)_
29. The runtime cache is `[uint64_t timestamp][module bytes]` with a zero
    timestamp as the failure sentinel. _(untested: `needs-unit-test`)_
30. The steps for adding a new built-in. _(untested: `process-doc`)_
31. The full intrinsic list is deliberately out of scope for this doc.
    _(untested: `internal-source-fact`)_
