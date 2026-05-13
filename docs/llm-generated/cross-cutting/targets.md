---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: aa9f598894b500cf2a462caaa7ca0e636d18394acbaf22fbc2f9064f2a7cf16c
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Targets, Capabilities, and Profiles

This document describes the supported compilation targets and the
capability and profile system that constrains code-generation
choices. The intended reader is a developer adding a new target,
debugging a target-specific codegen issue, or trying to understand
why a feature is rejected under a particular profile.

## Targets

A *target* is a (format, profile) pair. The set of formats Slang can
emit is determined by the emit backends linked into the compiler —
see [../pipeline/06-emit.md](../pipeline/06-emit.md) for the per-
backend details.

| Target | Output | Emit file |
| --- | --- | --- |
| HLSL | HLSL text (typically forwarded to DXC for DXIL) | [slang-emit-hlsl.cpp](../../../source/slang/slang-emit-hlsl.cpp) |
| GLSL | GLSL text (typically forwarded to glslang for SPIR-V) | [slang-emit-glsl.cpp](../../../source/slang/slang-emit-glsl.cpp) |
| SPIR-V (direct) | SPIR-V binary | [slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp) |
| Metal Shading Language | MSL text | [slang-emit-metal.cpp](../../../source/slang/slang-emit-metal.cpp) |
| WGSL | WGSL text | [slang-emit-wgsl.cpp](../../../source/slang/slang-emit-wgsl.cpp) |
| C++ shader | C++ text linked against `slang-rt` | [slang-emit-cpp.cpp](../../../source/slang/slang-emit-cpp.cpp) |
| CUDA | CUDA text | [slang-emit-cuda.cpp](../../../source/slang/slang-emit-cuda.cpp) |
| Torch glue | C++ pytorch binding | [slang-emit-torch.cpp](../../../source/slang/slang-emit-torch.cpp) |
| LLVM | Native code via `slang-llvm` | [slang-emit-llvm.cpp](../../../source/slang/slang-emit-llvm.cpp) |
| VM | Slang interpreter bytecode | [slang-emit-vm.cpp](../../../source/slang/slang-emit-vm.cpp) |
| Slang round-trip | Re-emit Slang source | [slang-emit-slang.cpp](../../../source/slang/slang-emit-slang.cpp) |

The full list of `SlangCompileTarget` enumerators visible in the
public API is in [include/slang.h](../../../include/slang.h); not
every public target value corresponds to a separate emit file (some
share a backend with format variations).

`SourceLanguage` (input flavor) is declared in
[slang-profile.h](../../../source/slang/slang-profile.h):

```cpp
enum class SourceLanguage : SlangSourceLanguageIntegral
{
    Unknown = SLANG_SOURCE_LANGUAGE_UNKNOWN,
    Slang   = SLANG_SOURCE_LANGUAGE_SLANG,
    HLSL    = SLANG_SOURCE_LANGUAGE_HLSL,
    GLSL    = SLANG_SOURCE_LANGUAGE_GLSL,
    C       = SLANG_SOURCE_LANGUAGE_C,
    CPP     = SLANG_SOURCE_LANGUAGE_CPP,
    CUDA    = SLANG_SOURCE_LANGUAGE_CUDA,
    SPIRV   = SLANG_SOURCE_LANGUAGE_SPIRV,
    Metal   = SLANG_SOURCE_LANGUAGE_METAL,
    WGSL    = SLANG_SOURCE_LANGUAGE_WGSL,
    LLVM    = SLANG_SOURCE_LANGUAGE_LLVM,
    CountOf = SLANG_SOURCE_LANGUAGE_COUNT_OF,
};
```

## Capability system

The capability system tracks features that a target supports. It is
declared in
[slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef)
and processed by `slang-capability-generator` (a build-time tool that
emits `slang-generated-capability-defs.h` and
`slang-generated-capability-defs-impl.h` consumed via the
`slang-capability-defs` and `slang-capability-lookup` targets in
[source/slang/CMakeLists.txt](../../../source/slang/CMakeLists.txt)).

### Vocabulary

From the comments at the top of
[slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef):

- A *capability atom* is the smallest unit; it represents one
  target / extension / hardware feature. Examples (paraphrased from
  the file): `_GL_EXT_ray_tracing` is a GLSL extension atom;
  `glsl` is a code-gen-target atom.
- A *capability name* is a Boolean expression — a disjunction of
  conjunctions of atoms. Example: `raytracing` expands to
  `glsl + _GL_EXT_ray_tracing | spirv_1_4 + SPV_KHR_ray_tracing | hlsl + _sm_6_4`.
- An *abstract* capability does not introduce an atom; it defines a
  "keyhole" that other atoms populate. `target` and `stage` are
  distinct keyholes; an atom derived directly from an abstract
  capability is a "key atom" for that keyhole.

### Definition forms

Three forms of declaration in `.capdef`:

- `def Foo;` introduces a new atom. With an inheritance clause the
  atom expands to all inherited atoms plus the new one.
- `abstract Foo;` introduces a keyhole; no real atom is emitted.
- `alias Foo = Bar;` introduces a name without introducing atoms.

The arithmetic of compatibility:

- `+` (conjunction) requires both operand sets be compatible. Two
  conjunctions are incompatible if they populate the same keyhole
  with different atoms (e.g. `hlsl + glsl` is incompatible because
  both populate the `target` keyhole).
- `|` (disjunction) creates an alternative; if its operand sets are
  incompatible the result is a disjunction (e.g. `hlsl | glsl`).
- An unpopulated keyhole means the set is compatible with any
  key atom of that keyhole (e.g. `vertex + glsl` works because
  `vertex` does not populate `target`).

### Runtime representation

The C++ side declares `Capability` and `CapabilitySet` in
[slang-capability.h](../../../source/slang/slang-capability.h);
implementation in
[slang-capability.cpp](../../../source/slang/slang-capability.cpp).
A `CapabilitySet` is the disjunction-of-conjunctions normal form
described above. Operations:

- Computing the join (intersection) of two capability sets — used
  when checking whether a function's required capabilities are
  satisfied by an entry-point's promised capabilities.
- Inferring the minimum capability requirement of a piece of IR —
  used by the `slang-ir-late-require-capability` pass, see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

The high-level design is described in
[../../design/capabilities.md](../../design/capabilities.md); this
document does not duplicate it.

### Auto-generated reference

A `///` doc-comment immediately preceding a `def` or `alias` in the
`.capdef` is harvested into the auto-generated capability reference
mentioned at the top of the file
(`a3-02-reference-capability-atoms.md`). Comments interrupted by a
plain `//` line are dropped (per the rules near the top of
[slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef)).

## Profiles

Profiles bundle a target choice with a feature level. The
declaration is in
[slang-profile.h](../../../source/slang/slang-profile.h);
implementation in
[slang-profile.cpp](../../../source/slang/slang-profile.cpp); the
table of profile names is in
[slang-profile-defs.h](../../../source/slang/slang-profile-defs.h)
(an X-macro included in several places).

A `Profile` carries:

- A `Stage` (compute, vertex, fragment, geometry, hull, domain,
  raytracing stages, mesh, amplification, ...).
- A `Family` and `Version` (e.g. HLSL Shader Model 6_6, GLSL 450).
- A target-language hint that the profile applies to.

Profiles map onto capability sets at the input to the back-end:
`-target spirv -profile glsl_450` produces a `CapabilitySet` that
has both `glsl_450` atoms and the `spirv` target-keyhole.

## How target choice affects IR

The IR itself is mostly target-agnostic. Two places where the target
shows through:

1. **Specialization passes**
   `slang-ir-specialize-target-switch.cpp` and
   `slang-ir-specialize-stage-switch.cpp` (see
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md))
   resolve `[target]` / `[stage]` conditional code paths against the
   active `TargetRequest`.
2. **Target-specific lowering passes** named by target acronym:
   - HLSL: [slang-ir-hlsl-legalize.cpp](../../../source/slang/slang-ir-hlsl-legalize.cpp)
   - GLSL: [slang-ir-glsl-legalize.cpp](../../../source/slang/slang-ir-glsl-legalize.cpp), [slang-ir-glsl-liveness.cpp](../../../source/slang/slang-ir-glsl-liveness.cpp)
   - SPIR-V: [slang-ir-spirv-legalize.cpp](../../../source/slang/slang-ir-spirv-legalize.cpp), [slang-ir-spirv-snippet.cpp](../../../source/slang/slang-ir-spirv-snippet.cpp)
   - Metal: [slang-ir-metal-legalize.cpp](../../../source/slang/slang-ir-metal-legalize.cpp)
   - WGSL: [slang-ir-wgsl-legalize.cpp](../../../source/slang/slang-ir-wgsl-legalize.cpp)
   - CUDA: [slang-ir-cuda-immutable-load.cpp](../../../source/slang/slang-ir-cuda-immutable-load.cpp), [slang-ir-lower-cuda-builtin-types.cpp](../../../source/slang/slang-ir-lower-cuda-builtin-types.cpp)
   - Vulkan: [slang-ir-vk-invert-y.cpp](../../../source/slang/slang-ir-vk-invert-y.cpp)
   - Torch: [slang-ir-pytorch-cpp-binding.cpp](../../../source/slang/slang-ir-pytorch-cpp-binding.cpp)

Some passes are conditional within a backend by inspecting the
profile (e.g. an HLSL Shader Model gate). The orchestrator that
picks which passes run is `linkAndOptimizeIR` in
[slang-emit.cpp](../../../source/slang/slang-emit.cpp).

## Per-target pass pipelines

For an ordered, control-flow-graph view of the IR passes that run
end-to-end for each shader target (Phase A link/prep → Phase B
specialization → Phase C target legalization → Phase D emit and
downstream tools), see the per-target pages under
[../target-pipelines/](../target-pipelines/):

- [../target-pipelines/index.md](../target-pipelines/index.md) —
  cross-target navigation hub with comparison table.
- [../target-pipelines/spirv.md](../target-pipelines/spirv.md) —
  SPIR-V direct-emit path.
- [../target-pipelines/hlsl.md](../target-pipelines/hlsl.md) —
  HLSL plus DXC / fxc downstream.
- [../target-pipelines/metal.md](../target-pipelines/metal.md) —
  Metal plus Apple `metal` downstream.
- [../target-pipelines/wgsl.md](../target-pipelines/wgsl.md) —
  WGSL plus Tint downstream.
- [../target-pipelines/cuda.md](../target-pipelines/cuda.md) —
  CUDA plus nvrtc downstream.

This page (`cross-cutting/targets.md`) describes the per-target
options, capability sets, and predicate functions; the
`target-pipelines/` pages describe the ordered pass sequence.

## Adding a new target

The full checklist:

1. **Public API.** Add a new `SlangCompileTarget` enumerator to
   [include/slang.h](../../../include/slang.h). Per the public-
   header rules in [CLAUDE.md](../../../CLAUDE.md), append the new
   value before the terminal count sentinel and assign it an
   explicit integer.
2. **Emit backend.** Add `slang-emit-<target>.{h,cpp}` under
   [source/slang/](../../../source/slang/). For a textual target,
   subclass `CLikeSourceEmitter` from
   [slang-emit-c-like.h](../../../source/slang/slang-emit-c-like.h).
3. **Dispatcher.** Wire the backend into
   [slang-emit.cpp](../../../source/slang/slang-emit.cpp) — both
   the `#include` and the dispatch logic in
   `emitEntryPointsSourceFromIR`.
4. **Prelude.** If the emitted code requires runtime support, add a
   prelude header under [prelude/](../../../prelude/) and emit a
   `#include` for it from the backend.
5. **Capability atoms.** Add atoms to
   [slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef)
   so the front-end can reject features the new target does not
   support.
6. **Target-specific IR passes.** If the target needs custom
   legalization, add `slang-ir-<target>-legalize.cpp` and gate it on
   the `TargetRequest`. See
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
7. **Profile entries.** If the new target supports versioned
   profiles, extend
   [slang-profile-defs.h](../../../source/slang/slang-profile-defs.h)
   and the supporting tables in
   [slang-profile.cpp](../../../source/slang/slang-profile.cpp).
8. **Tests.** Add fixtures under [tests/](../../../tests/),
   including HLSL/GLSL parity tests where applicable. See
   [CLAUDE.md](../../../CLAUDE.md) for the test-directive
   conventions.
9. **User-guide updates.** A new target is a user-visible feature;
   add an entry under [docs/user-guide/](../../user-guide/).

## What is not in this document

- The full list of capability atoms — it lives in
  [slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef)
  and the auto-generated reference page.
- The detailed profile-version table — it lives in
  [slang-profile-defs.h](../../../source/slang/slang-profile-defs.h).
- The user-facing target documentation — see
  [../../user-guide/](../../user-guide/) and
  [../../command-line-slangc-reference.md](../../command-line-slangc-reference.md).
