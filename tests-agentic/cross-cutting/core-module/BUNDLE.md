---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:55:51Z
source_commit: bbd84dc65e58598bfa71fafe72764b4076b0869b
watched_paths_digest: 79f0d532a4fd57a48ed9dba066f683cc3eb4cbf8e598a4baa37929c3e9113855
source_doc: docs/llm-generated/cross-cutting/core-module.md
source_doc_digest: 940b46557a03f1e089bcd47c43fb6d5a5cc72aa0dcf703ab83e117d28307a902
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/core-module

## Intent

Tests verify the core-module / GLSL-module / standard-module / prelude
claims in
[`docs/llm-generated/cross-cutting/core-module.md`](../../../docs/llm-generated/cross-cutting/core-module.md):

- The core module is a single identified Slang module
  (`public module core;`) whose declarations (scalar typedef aliases,
  vector/matrix types, `Optional`, `Tuple`) are in scope without
  explicit `import`.
- The HLSL meta-module layers in HLSL-named intrinsics (`dot`,
  `length`, `mul`) and resource types (`Texture2D`,
  `RWStructuredBuffer`) that lower to per-target spellings.
- The diff meta-module declares `IDifferentiable` and is implicitly
  loaded.
- The GLSL module (`vec3`, `gl_*`) is loaded conditionally via
  `-allow-glsl` (the doc's "compiler pulls it in when ... asks for
  GLSL-flavoured names from Slang code" wording).
- Standard modules (`import slang.<name>`; currently `neural`) are
  loaded on demand at runtime, NOT implicit.
- Per-target preludes (the C/C++/CUDA headers) are referenced from
  emitted text output, observable via prelude-defined markers
  (`SLANG_PRELUDE_EXPORT`, `extern "C" __global__`).

Multi-target where the claim is target-dependent (intrinsic /
resource / vector lowering); single-target where the claim is
module-identity (typedef resolves, `Optional<T>` compiles).

## Claims enumerated

| Claim ID | Anchor                          | Claim (one line)                                                                                  | Tests                                                  |
| -------- | ------------------------------- | ------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| C-01     | `#what-the-core-module-provides`| `int32_t` typedef resolves without explicit import and lowers to 32-bit signed int per target.    | `core-typedef-int32-resolves.slang`                    |
| C-02     | `#what-the-core-module-provides`| `uint32_t` typedef resolves and lowers to 32-bit unsigned int per target.                         | `core-typedef-uint32-resolves.slang`                   |
| C-03     | `#what-the-core-module-provides`| `float32_t` typedef resolves and lowers to a 32-bit float per target.                             | `core-typedef-float32-resolves.slang`                  |
| C-04     | `#what-the-core-module-provides`| `float64_t` typedef resolves and lowers to a 64-bit float on HLSL and CUDA.                       | `core-typedef-float64-resolves.slang`                  |
| C-05     | `#what-the-core-module-provides`| Core vector and matrix types (`float3`, `float3x3`) lower to per-target spellings.                | `core-vector-matrix-types-lower.slang`                 |
| C-06     | `#what-the-core-module-provides`| `Optional<T>` is a core-module type usable without explicit import.                                | `core-optional-type-usable.slang`                      |
| C-07     | `#core-module`                  | HLSL meta-module's `dot` intrinsic lowers per target (`dot` / `dot` / `OpDot`).                   | `hlsl-intrinsic-dot-lowers-per-target.slang`           |
| C-08     | `#core-module`                  | HLSL meta-module's `length` intrinsic lowers per target (`length` / `length` / `Length`).         | `hlsl-intrinsic-length-lowers-per-target.slang`        |
| C-09     | `#core-module`                  | HLSL meta-module's `mul(matrix, vector)` lowers per target (`mul` / `*` / `OpVectorTimesMatrix`). | `hlsl-intrinsic-mul-matrix-lowers-per-target.slang`    |
| C-10     | `#core-module`                  | HLSL meta-module's `RWStructuredBuffer<T>` lowers to per-target binding shapes.                   | `hlsl-resource-rwstructuredbuffer-lowers-per-target.slang` |
| C-11     | `#core-module`                  | HLSL meta-module's `Texture2D` lowers per target (HLSL keeps name, GLSL/SPIR-V emit image type).  | `hlsl-resource-texture2d-lowers-per-target.slang`      |
| C-12     | `#core-module`                  | Diff meta-module declares `IDifferentiable`; a conforming struct is accepted by the front-end.    | `diff-idifferentiable-frontend-accepts.slang`          |
| C-13     | `#glsl-module`                  | GLSL module supplies `vec3` alias when loaded via `-allow-glsl`; lowers to per-target spellings.  | `glsl-module-vec3-alias.slang`                         |
| C-14     | `#preludes`                     | C++ prelude is referenced from emitted CPP output (`SLANG_PRELUDE_EXPORT` marker on entry point). | `cpp-prelude-export-marker.slang`                      |
| C-15     | `#preludes`                     | CUDA prelude is referenced from emitted CUDA output (`extern "C"` + `__global__` on entry point). | `cuda-prelude-extern-c-marker.slang`                   |
| C-16     | `#standard-modules`             | Standard module `neural` is NOT implicitly imported; `neural`-namespaced names are unresolved.    | `standard-module-neural-not-implicit.slang`            |

## Tests in this bundle

| File                                                       | Intent     | Doc anchor                       |
| ---------------------------------------------------------- | ---------- | -------------------------------- |
| `core-optional-type-usable.slang`                          | functional | `#what-the-core-module-provides` |
| `core-typedef-float32-resolves.slang`                      | functional | `#what-the-core-module-provides` |
| `core-typedef-float64-resolves.slang`                      | functional | `#what-the-core-module-provides` |
| `core-typedef-int32-resolves.slang`                        | functional | `#what-the-core-module-provides` |
| `core-typedef-uint32-resolves.slang`                       | functional | `#what-the-core-module-provides` |
| `core-vector-matrix-types-lower.slang`                     | functional | `#what-the-core-module-provides` |
| `cpp-prelude-export-marker.slang`                          | functional | `#preludes`                      |
| `cuda-prelude-extern-c-marker.slang`                       | functional | `#preludes`                      |
| `diff-idifferentiable-frontend-accepts.slang`              | functional | `#core-module`                   |
| `glsl-module-vec3-alias.slang`                             | functional | `#glsl-module`                   |
| `hlsl-intrinsic-dot-lowers-per-target.slang`               | functional | `#core-module`                   |
| `hlsl-intrinsic-length-lowers-per-target.slang`            | functional | `#core-module`                   |
| `hlsl-intrinsic-mul-matrix-lowers-per-target.slang`        | functional | `#core-module`                   |
| `hlsl-resource-rwstructuredbuffer-lowers-per-target.slang` | functional | `#core-module`                   |
| `hlsl-resource-texture2d-lowers-per-target.slang`          | functional | `#core-module`                   |
| `standard-module-neural-not-implicit.slang`                | negative   | `#standard-modules`              |
| `dot-zero-vector.slang`                                    | boundary   | `#core-module`                   |
| `dot-inf-vector.slang`                                     | boundary   | `#core-module`                   |
| `dot-nan-vector.slang`                                     | boundary   | `#core-module`                   |
| `length-zero-vector.slang`                                 | boundary   | `#core-module`                   |
| `length-nan-vector.slang`                                  | boundary   | `#core-module`                   |
| `normalize-zero-vector.slang`                              | boundary   | `#core-module`                   |
| `mul-matrix-vector-2x2.slang`                              | boundary   | `#core-module`                   |
| `mul-matrix-vector-4x4.slang`                              | boundary   | `#core-module`                   |
| `mul-zero-matrix-vector.slang`                             | boundary   | `#core-module`                   |
| `core-typedef-int32-max.slang`                             | boundary   | `#what-the-core-module-provides` |
| `core-typedef-uint32-max.slang`                            | boundary   | `#what-the-core-module-provides` |
| `core-typedef-float32-inf.slang`                           | boundary   | `#what-the-core-module-provides` |
| `core-typedef-float32-nan.slang`                           | boundary   | `#what-the-core-module-provides` |
| `conv-int32-max-to-float32.slang`                          | boundary   | `#what-the-core-module-provides` |
| `conv-half-roundtrip.slang`                                | boundary   | `#what-the-core-module-provides` |
| `printf-int-max.slang`                                     | boundary   | `#core-module`                   |
| `printf-int-min.slang`                                     | boundary   | `#core-module`                   |
| `printf-float-inf.slang`                                   | boundary   | `#core-module`                   |
| `printf-float-nan.slang`                                   | boundary   | `#core-module`                   |
| `printf-no-args.slang`                                     | boundary   | `#core-module`                   |
| `printf-many-args.slang`                                   | stress     | `#core-module`                   |
| `texture-sample-uv-center.slang`                           | boundary   | `#core-module`                   |
| `texture-sample-uv-corner.slang`                           | boundary   | `#core-module`                   |
| `glsl-mat4-allow-glsl.slang`                               | boundary   | `#glsl-module`                   |
| `glsl-min-max-clamp-aliases.slang`                         | boundary   | `#glsl-module`                   |
| `import-nonexistent-module.slang`                          | negative   | `#standard-modules`              |
| `glsl-vec3-without-allow-glsl.slang`                       | negative   | `#glsl-module`                   |
| `import-diff-by-name-fails.slang`                          | negative   | `#standard-modules`              |
| `dot-many-uses-stress.slang`                               | stress     | `#core-module`                   |
| `mul-many-uses-stress.slang`                               | stress     | `#core-module`                   |
| `optional-deep-nesting-stress.slang`                       | stress     | `#what-the-core-module-provides` |

## Doc gaps observed

- `#core-module` lists `__intrinsic_op` and `__target_intrinsic` as
  the modifier vocabulary mapping core-module declarations onto IR
  and per-target spellings, but the per-target spelling table for
  any specific intrinsic (e.g. `dot`, `length`, `mul`) is not
  enumerated in `cross-cutting/core-module.md` itself — tests had
  to discover SPIR-V spellings (`OpDot`, `Length`,
  `OpVectorTimesMatrix`) by inspection.
- `#glsl-module` says the GLSL module is "target-conditional ...
  when the user is compiling GLSL or asks for GLSL-flavoured names
  from Slang code" but does not name the user-facing flag
  (`-allow-glsl`) that enables the second mode from Slang source.
- `#standard-modules` describes the on-demand `import slang.<name>`
  mechanism but does not describe what happens when a standard-
  module-namespaced name is referenced without the import — i.e.
  the doc does not state the negative case used by
  `standard-module-neural-not-implicit.slang`.
- `#preludes` lists prelude headers by file name but does not
  describe which symbols from each prelude appear in emitted
  output. Tests had to find observable markers
  (`SLANG_PRELUDE_EXPORT`, `extern "C" __global__`) by inspection.
- `#what-the-core-module-provides` lists `Optional`, `Result`,
  `Tuple` but does not describe their public API
  (`hasValue` property, `value` property, `none` literal).
- 16-bit float support (`float16_t` / `half`) is mentioned in the
  doc's typedef list but its per-target capability requirements
  (e.g. HLSL `-profile sm_6_2`, SPIR-V `Float16` capability) are
  not documented; we therefore skip a `float16_t` lowering test
  rather than guess at the right flag set.
- The doc lists `dot`, `length`, `mul` (and by inference
  `normalize`) as HLSL-meta-module intrinsics but does not name the
  documented behaviour of these intrinsics at numeric edges
  (zero-vector, infinity, NaN) — boundary tests therefore assert
  only front-end acceptance and emit preservation, not specific
  per-target runtime values.
- The doc does not state the per-target spelling of `printf`
  format conversions (`%d`, `%f` at MIN/MAX/inf/NaN) or whether
  `slangi` shares the host's libc formatting; boundary printf
  tests cite the doc's general "HLSL meta-module exposes ...
  intrinsics" wording and assert string-match on the interpreter
  output observed today.
- The doc identifies the GLSL module as "target-conditional" but
  does not enumerate the GLSL-flavoured arithmetic helpers
  (`min`, `max`, `clamp`, etc.) that the module re-exports — our
  `glsl-min-max-clamp-aliases` test had to discover their
  availability empirically.
- The doc does not state that the diff meta-module is non-
  importable by `import diff;` from user code — our negative
  `import-diff-by-name-fails` test had to discover the
  loader-search-path behaviour empirically.

## Out of scope (no-GPU runner)

- **Atomic-op intrinsics** (e.g. `InterlockedAdd`, `atomicAdd`,
  `OpAtomicIAdd`) — target-specific spellings, and prior agentic
  attempts have wasted iterations on them; skipped entirely.
- The `SLANG_EMBED_CORE_MODULE` ON vs. OFF build option — affects
  how the compiler ships the module, not what user code sees.
- The byte-for-byte embedded module artefact (a build product).
- The `slang-standard-module-config.h.in` template machinery
  (internal build infrastructure).
- The full intrinsic list (the doc explicitly disclaims
  enumerating it).
- The "Adding a new built-in" developer workflow (not a user-
  observable behaviour).
- DXIL / metallib / WGSL→SPIRV downstream paths and Torch / LLVM /
  Slang round-trip targets (no-GPU runner; no downstream toolchains).
