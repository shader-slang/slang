---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T16:39:21Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 720cbadffe0ddbcfd07c03b208f3f7cbad55f384b2abb3ca09da30eb7d155f95
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

Rows below group the public `SlangCompileTarget` values declared in
[include/slang.h](../../../../include/slang.h) by the emit backend that
produces them. Several enumerators correspond to format variations
(text vs binary vs assembly, shader vs host) that flow through the
same emit file and are dispatched by `CodeGenTarget` further down
the pipeline.

| Target group | Public `SlangCompileTarget` values | Output | Emit file(s) |
| --- | --- | --- | --- |
| HLSL | `SLANG_HLSL`, `SLANG_DXBC`, `SLANG_DXBC_ASM`, `SLANG_DXIL`, `SLANG_DXIL_ASM` | HLSL text plus downstream DXBC/DXIL produced via FXC / DXC | [slang-emit-hlsl.cpp](../../../../source/slang/slang-emit-hlsl.cpp) (DXBC/DXIL are downstream-compiled) |
| GLSL | `SLANG_GLSL`, plus the retained-but-removed `SLANG_GLSL_VULKAN_DEPRECATED` and `SLANG_GLSL_VULKAN_ONE_DESC_DEPRECATED` enumerators | GLSL text (typically forwarded to glslang for SPIR-V) | [slang-emit-glsl.cpp](../../../../source/slang/slang-emit-glsl.cpp) |
| SPIR-V (direct) | `SLANG_SPIRV`, `SLANG_SPIRV_ASM` | SPIR-V binary or assembly | [slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp) |
| Metal Shading Language | `SLANG_METAL`, `SLANG_METAL_LIB`, `SLANG_METAL_LIB_ASM` | MSL text, Metal library, Metal library assembly | [slang-emit-metal.cpp](../../../../source/slang/slang-emit-metal.cpp) (`*_LIB`/`*_LIB_ASM` go through Metal's downstream tools) |
| WGSL | `SLANG_WGSL`, `SLANG_WGSL_SPIRV`, `SLANG_WGSL_SPIRV_ASM` | WGSL text, plus SPIR-V binary/assembly produced via WGSL | [slang-emit-wgsl.cpp](../../../../source/slang/slang-emit-wgsl.cpp) |
| C++ shader | `SLANG_CPP_SOURCE`, `SLANG_C_SOURCE`, `SLANG_CPP_HEADER` | C/C++ text linked against `slang-rt`; header variant emits a declarations-only file | [slang-emit-cpp.cpp](../../../../source/slang/slang-emit-cpp.cpp) |
| C++ host | `SLANG_HOST_CPP_SOURCE` | Host-side C++ source | [slang-emit-cpp.cpp](../../../../source/slang/slang-emit-cpp.cpp) |
| CUDA | `SLANG_CUDA_SOURCE`, `SLANG_PTX`, `SLANG_CUDA_OBJECT_CODE`, `SLANG_CUDA_HEADER` | CUDA text, PTX, object code, header | [slang-emit-cuda.cpp](../../../../source/slang/slang-emit-cuda.cpp) (PTX and object code via NVRTC / nvcc) |
| Torch glue | `SLANG_CPP_PYTORCH_BINDING` | C++ PyTorch binding | [slang-emit-torch.cpp](../../../../source/slang/slang-emit-torch.cpp) |
| CPU binaries / host-callable | `SLANG_HOST_HOST_CALLABLE`, `SLANG_SHADER_HOST_CALLABLE`, `SLANG_HOST_OBJECT_CODE`, `SLANG_OBJECT_CODE`, `SLANG_HOST_SHARED_LIBRARY`, `SLANG_SHADER_SHARED_LIBRARY`, `SLANG_HOST_EXECUTABLE`, `SLANG_HOST_LLVM_IR`, `SLANG_SHADER_LLVM_IR` | LLVM-IR, object code, JIT-callable code, shared libraries, executables | [slang-emit-llvm.cpp](../../../../source/slang/slang-emit-llvm.cpp) via `emitLLVMForEntryPoints` when `isCPUTargetViaLLVM`; otherwise routed to a downstream C++ compiler from [slang-code-gen.cpp](../../../../source/slang/slang-code-gen.cpp) |
| VM | `SLANG_HOST_VM` | Slang interpreter bytecode | [slang-emit-vm.cpp](../../../../source/slang/slang-emit-vm.cpp) |
| Slang round-trip | (no public target and no `CodeGenTarget` value) | Unimplemented stub: `emitSlangDeclarationsForEntryPoints` ignores its inputs and writes no source | [slang-emit-slang.cpp](../../../../source/slang/slang-emit-slang.cpp) |

`SLANG_TARGET_UNKNOWN` and `SLANG_TARGET_NONE` are sentinel values
that do not select an emit backend. `SLANG_TARGET_COUNT_OF` is the
enumerator terminator and is not a usable target.

`SourceLanguage` (input flavor) is declared in
[slang-profile.h](../../../../source/slang/slang-profile.h):

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
[slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef)
and processed by `slang-capability-generator` (a build-time tool that
emits `slang-generated-capability-defs.h` and
`slang-generated-capability-defs-impl.h` consumed via the
`slang-capability-defs` and `slang-capability-lookup` targets in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)).

### Vocabulary

From the comments at the top of
[slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef):

- A *capability atom* is the smallest unit; it represents one
  target / extension / hardware feature. Examples (paraphrased from
  the file): `_GL_EXT_ray_tracing` is a GLSL extension atom;
  `glsl` is a code-gen-target atom.
- A *capability name* is a Boolean expression — a disjunction of
  conjunctions of atoms. Example: `raytracing` expands to
  `GL_EXT_ray_tracing | _sm_6_3 | cuda`.
- An *abstract* capability does not introduce an atom; it defines a
  "keyhole" that other atoms populate. `target` and `stage` are
  distinct keyholes; an atom derived directly from an abstract
  capability is a "key atom" for that keyhole.
- A *version family* is a chain of atoms ordered by inheritance that
  express successive versions of one target — the Shader Model chain
  (`_sm_4_0` ... `_sm_6_10`), the GLSL chain (`_GLSL_130` ...
  `_GLSL_460`), the SPIR-V chain, and the MetalLib chain. Membership
  is tested by `isTargetVersionAtom` (any family) and the per-family
  `isSpirvVersionAtom` in
  [slang-capability.h](../../../../source/slang/slang-capability.h).
- A name whose spelling begins with `_` is *internal*: it is a
  building block that user code is not expected to name directly.
  `isInternalCapabilityName` in
  [slang-capability.cpp](../../../../source/slang/slang-capability.cpp)
  is just a leading-underscore test. Most public atoms are a
  non-underscored `alias` over one or more internal `def`s.

### Definition forms

Three forms of declaration in `.capdef`:

- `def Foo;` introduces a new atom. With an inheritance clause the
  atom expands to all inherited atoms plus the new one.
- `abstract Foo;` introduces a keyhole; no real atom is emitted.
- `alias Foo = Bar;` introduces a name without introducing atoms.

Each version family carries a `*_latest` alias that names its
highest version, so that call sites and tests can say "newest" once
instead of chasing a version bump through the file. The public and
internal spellings are kept in step:

```
alias _sm_latest    = _sm_6_10;   alias sm_latest    = _sm_6_10;
alias _GLSL_latest  = _GLSL_460;  alias GLSL_latest  = _GLSL_460;
alias _spirv_latest = _spirv_1_6; alias spirv_latest = _spirv_1_6;
alias metallib_latest = metallib_4_0;
```

C++ reaches the same values through the `getLatest*Atom()` accessors
in [slang-capability.h](../../../../source/slang/slang-capability.h)
— `getLatestSpirvAtom`, `getLatestMetalAtom`, `getLatestHlslAtom`,
and `getLatestGlslAtom`. The HLSL and GLSL accessors exist so that
the version-family range tests can be written as a bounded compare
(`name >= CapabilityAtom::_sm_4_0 && name <= getLatestHlslAtom()`)
rather than an enumerated list that must be edited on every new
Shader Model.

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

Inheritance is also how an extension records its *version floor*, and
the SPIR-V atoms show the layering. An extension atom derives from
the earliest target version that can host it
(`def SPV_EXT_shader_64bit_indexing : _spirv_1_0;`), and the SPIR-V
`OpCapability` it enables derives from the extension atom in turn
(`def spvShader64BitIndexingEXT : SPV_EXT_shader_64bit_indexing;`),
so asking for the capability transitively asks for the extension and
the floor. Because the floor is inherited rather than asserted
separately, giving two spellings of one feature the same floor keeps
them interchangeable: `SPV_KHR_physical_storage_buffer` is the
KHR-promoted name of `SPV_EXT_physical_storage_buffer` and both
derive from `_spirv_1_3`, so a requirement written in terms of the
KHR name — as `SPV_EXT_shader_invocation_reorder` is
(`_spirv_1_4 + SPV_KHR_ray_tracing + SPV_KHR_physical_storage_buffer`)
— does not push the effective version up. That matters because a
raised floor is user-visible; see the `-capability` discussion under
[Profiles](#profiles). The raise shows up in the emitted module:
because `SPV_KHR_cooperative_matrix` derives from `_spirv_1_6 +
SPV_EXT_physical_storage_buffer + SPV_KHR_vulkan_memory_model`,
`-target spirv-asm -capability SPV_KHR_cooperative_matrix` emits
`; Version: 1.6` and `OpCapability VulkanMemoryModel` for a kernel
that uses neither.

### Runtime representation

The C++ side declares `Capability` and `CapabilitySet` in
[slang-capability.h](../../../../source/slang/slang-capability.h);
implementation in
[slang-capability.cpp](../../../../source/slang/slang-capability.cpp).
A `CapabilitySet` is the disjunction-of-conjunctions normal form
described above. Operations:

- Computing the join (intersection) of two capability sets — used
  when checking whether a function's required capabilities are
  satisfied by an entry-point's promised capabilities.
- Inferring the minimum capability requirement of a piece of IR —
  used by the `slang-ir-late-require-capability` pass, see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
- Classifying a single atom, so that callers can ask what kind of
  thing an atom is without hard-coding names:
  `isDirectChildOfAbstractAtom`, `isStageAtom`, `isTargetVersionAtom`,
  `isSpirvVersionAtom`, `isSpirvExtensionAtom`, and `hasTargetAtom`.
  `getAtomSetOfTargets` and `getAtomSetOfStages` return the
  populated key-atom set for each of the two keyholes.

Whether a shortfall in that join is fatal is a command-line choice:
`maybeDiagnoseWarningOrError` in
[slang-compiler.h](../../../../source/slang/slang-compiler.h) selects
a `Capability`-category diagnostic's error form under
`-restrictive-capability-check` and its warning form otherwise, and
`maybeDiagnose` drops the diagnostic entirely under
`-ignore-capabilities`. For a missing atom those two forms are
"entry point uses capabilities not in specified profile" and
"profile implicitly upgraded", the second widening the profile to
include the missing atoms and continuing.

The high-level design is described in
[../../../design/capabilities.md](../../../design/capabilities.md); this
document does not duplicate it.

### Auto-generated reference

Every `def` and `alias` is a *documentable atom*. A run of `///`
lines immediately preceding one is harvested into
[a4-02-reference-capability-atoms.md](../../../user-guide/a4-02-reference-capability-atoms.md);
a plain `//` line in the middle of the run truncates it. A line of
the form `/// [GROUP]` selects the reference-page heading the entry
files under, and only six groups are accepted: `[Target]`,
`[Stage]`, `[EXT]`, `[Version]`, `[Compound]`, `[Other]`. The rules
are spelled out in the header comment of
[slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef).

Because a feature is usually spelled as an internal `def` plus a
public `alias`, the doc-comment must sit on the **public alias** for
the description to appear under the name a user would write. The
work-graph node stage is the model case: the bare
`def _node : stage;` carries no comment, and the documentation
lives on `alias node = _node + _sm_6_8;` under `/// [Stage]`.

The reference page is generated, never hand-edited. To regenerate
it, build the `slang-capability-generator` target and run it with
the `.capdef` as input plus `--target-directory` and
`--doc <output.md>`; commit the edited `.capdef` and the
regenerated markdown together.

Aliases tagged `[Compound]` in those comments are the names the
front-end uses to gate a user-visible builtin or operation against
the active target/stage. For example `abort` expands to
`GL_EXT_shader_abort` (GLSL `abortEXT` / SPIR-V `OpAbortKHR`) and
gates the variadic
`void abort<each T>(NativeString format, expand each T args)`
builtin, whose whole user surface is a call such as
`abort("bad value: %u", v);`.
`rayquery_sphere_nv` / `rayquery_lss_nv` each disjoin the per-target
support for the NV sphere / linear-swept-spheres ray-query accessors
(GLSL `_GL_NV_linear_swept_spheres`, HLSL/NVAPI `_sm_6_3`, or SPIR-V
`spvRayQueryKHR` combined with the matching geometry capability), and
gate ten `RayQuery` methods — the `Candidate` / `Committed` pairs of
`SphereObjectPositionAndRadiusNV`, `IsNonOpaqueSphereNV` /
`IsSphereNV`, `LssObjectPositionsAndRadiiNV`, `LssHitParameterNV`
and `IsNonOpaqueLssNV` / `IsLssNV` — each of which also carries
`[__requiresNVAPI]` for its HLSL arm.
The restriction of `subgroup_workgroup_index` (the `WaveGetWaveIndex` /
`WaveGetNumWaves` queries) to compute-class stages on GLSL / SPIR-V is
likewise encoded as a compound alias, so misuse is rejected by the
capability system rather than producing invalid output.

## Profiles

A profile pins a stage and a feature-level version; it does not
carry a target format. The declaration is in
[slang-profile.h](../../../../source/slang/slang-profile.h);
implementation in
[slang-profile.cpp](../../../../source/slang/slang-profile.cpp); the
table of profile names is in
[slang-profile-defs.h](../../../../source/slang/slang-profile-defs.h)
(an X-macro included in several places).

A `Profile` carries:

- A `Stage`, declared by the `PROFILE_STAGE` X-macro rows in
  [slang-profile-defs.h](../../../../source/slang/slang-profile-defs.h)
  (compute, vertex, fragment, geometry, hull, domain, the raytracing
  stages, mesh, amplification, dispatch, and the work-graph `node`
  stage).
- A `Version` (e.g. HLSL Shader Model 6_6, GLSL 450). The stage and
  the version are the only state packed into `Profile::raw`;
  `getFamily()` derives the `ProfileFamily` from the version rather
  than storing it. The pairing of a profile with an output format
  lives elsewhere — in the public `TargetDesc`, whose `format` and
  `profile` fields sit side by side in
  [include/slang.h](../../../../include/slang.h), and in the
  `TargetRequest` built from it.

Profiles map onto capability sets at the input to the back-end, but
the target keyhole is chosen first and profile atoms are admitted
only where they are compatible. `TargetRequest::getTargetCaps` in
[slang-target.cpp](../../../../source/slang/slang-target.cpp) adds
the target's own atom, then filters the profile's set: on the direct
SPIR-V path (`-target spirv` with `shouldEmitSPIRVDirectly()`) it
copies only the SPIR-V version and extension atoms out of the
profile and falls back to `spirv_1_5` when the profile supplies
none; on the SPIR-V-via-GLSL path it selects `glsl` instead and
re-expresses the profile's SPIR-V version as a GLSL one. A final
`join` of the whole profile set happens only when that set is
already implied by the target set, so a cross-family request such as
`-target spirv -profile glsl_450` does not yield a set holding both
`glsl_450` and `spirv` — the GLSL version atoms are dropped on the
direct path, and the via-GLSL path keeps `glsl` rather than `spirv`
as its target keyhole.

"Supplies none" is narrower than it reads. The capability alias a
profile version maps onto carries a SPIR-V disjunct of its own —
`sm_6_0` reaches `spirv_1_3` through `sm_6_0_version`, and `GLSL_450`
lists `spirv_1_3` directly — so an explicit `-profile` normally does
supply a SPIR-V version atom even when it belongs to another family.
The `spirv_1_5` fallback therefore applies to a compile with no
`-profile` at all, whose emitted header reads `; Version: 1.5`;
`-profile sm_6_0` and `-profile glsl_450` both emit `; Version: 1.3`.

The stage side of a profile and the stage keyhole of the capability
system are separate vocabularies that have to agree. `Stage::Node`
and the capability alias `node` are the paired spellings for
work-graph entry points; the alias is `_node + _sm_6_8`, so naming
the stage in capability terms also asserts the Shader Model floor
the stage needs. That floor arrives through the profile's own set —
`Profile::getCapabilityName` adds `CapabilityName::node` for
`Stage::Node` — so it raises what a node entry point *promises*
rather than being something the `-profile` version is checked
against: a `[shader("node")]` entry point at `-profile lib_6_6` is
not diagnosed for a missing `sm_6_8`.

### Profiles versus explicit `-capability`

`-profile` pins a version within a version family; `-capability`
adds atoms on top of it. Because many capability atoms inherit from
a version atom, an added capability can silently *raise* the emitted
target version above what the profile asked for. The option parser
detects that case rather than letting the two options disagree
quietly:
`doRequestedCapabilitiesRaiseTargetVersionAboveProfile` in
[slang-capability.cpp](../../../../source/slang/slang-capability.cpp)
takes the profile's `CapabilitySet`, the list of requested
`-capability` names, and the version family in question, and folds
the requested capabilities in one at a time using the same
compatibility guard as `TargetRequest::getTargetCaps` in
[slang-target.cpp](../../../../source/slang/slang-target.cpp). Atoms
are folded individually and not pre-joined, because a single
incompatible atom would invalidate the whole set and hide a
compatible atom's version raise. If the resulting highest version in
the family exceeds the pinned one, the call site in
[slang-options.cpp](../../../../source/slang/slang-options.cpp)
reports `conflicting-explicit-capability-and-profile`, defined in
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua).
When the profile pins no version of that family, the function
returns false and nothing is diagnosed.

The two options do not have separate diagnostic name spaces. An
unrecognised `-capability` atom is rejected by `findCapabilityName`
but reported as `unknown profile '<name>'` — the same diagnostic an
unrecognised `-profile` gets from `Profile::lookUp` — because the
`-capability` handler in
[slang-options.cpp](../../../../source/slang/slang-options.cpp)
reuses it rather than raising one of its own.

## How target choice affects IR

The IR itself is mostly target-agnostic. Two places where the target
shows through:

1. **Specialization passes**
   `slang-ir-specialize-target-switch.cpp` and
   `slang-ir-specialize-stage-switch.cpp` (see
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md))
   resolve `[target]` / `[stage]` conditional code paths against the
   active `TargetRequest`. The user-level syntax they resolve is
   `__target_switch`, used throughout
   [core.meta.slang](../../../../source/slang/core.meta.slang); its
   case labels are capability atom names, arms fall through, and
   `default:` covers every target with no arm of its own:

   ```slang
   __target_switch
   {
   case hlsl:  return 1;
   case glsl:
   case spirv: return 2;
   default:    return 99;
   }
   ```

   With no matching arm and no `default:`, the function has no body
   for the active target, and what is rejected is the entry point
   that reaches it rather than the switch.
2. **Target-specific lowering passes** named by target acronym:
   - HLSL: [slang-ir-hlsl-legalize.cpp](../../../../source/slang/slang-ir-hlsl-legalize.cpp)
   - GLSL: [slang-ir-glsl-legalize.cpp](../../../../source/slang/slang-ir-glsl-legalize.cpp), [slang-ir-glsl-liveness.cpp](../../../../source/slang/slang-ir-glsl-liveness.cpp)
   - SPIR-V: [slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp), [slang-ir-spirv-snippet.cpp](../../../../source/slang/slang-ir-spirv-snippet.cpp)
   - Metal: [slang-ir-metal-legalize.cpp](../../../../source/slang/slang-ir-metal-legalize.cpp)
   - WGSL: [slang-ir-wgsl-legalize.cpp](../../../../source/slang/slang-ir-wgsl-legalize.cpp)
   - CUDA: [slang-ir-cuda-immutable-load.cpp](../../../../source/slang/slang-ir-cuda-immutable-load.cpp), [slang-ir-lower-cuda-builtin-types.cpp](../../../../source/slang/slang-ir-lower-cuda-builtin-types.cpp)
   - Vulkan: [slang-ir-vk-invert-y.cpp](../../../../source/slang/slang-ir-vk-invert-y.cpp)
   - Torch: [slang-ir-pytorch-cpp-binding.cpp](../../../../source/slang/slang-ir-pytorch-cpp-binding.cpp)

Some passes are conditional within a backend by inspecting the
profile (e.g. an HLSL Shader Model gate). The orchestrator that
picks which passes run is `linkAndOptimizeIR` in
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp).

## Per-target pass pipelines

For an ordered, control-flow-graph view of the IR passes that run
end-to-end for each shader target (Phase A link/prep → Phase B
specialization → Phase C target legalization → Phase D emit and
downstream tools), see the per-target pages under
[../target-pipelines/](../target-pipelines):

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

The division of labor is: this page owns the *model* — what a target
is, how capability atoms and profiles are declared and combined, and
which knobs the front-end consults before any backend runs. The
`target-pipelines/` pages own the *per-target behavior* — the ordered
pass sequence, the gates that select each pass, the downstream tool
chain, and any emitter-level decision specific to one target. A
statement of the form "on SPIR-V, construct X is emitted as Y"
belongs there, not here, even when the reason is a capability.

## Adding a new target

The full checklist:

1. **Public API.** Add a new `SlangCompileTarget` enumerator to
   [include/slang.h](../../../../include/slang.h). Per the public-
   header rules in [CLAUDE.md](../../../../CLAUDE.md), append the new
   value before the terminal count sentinel and assign it an
   explicit integer.
2. **Emit backend.** Add `slang-emit-<target>.{h,cpp}` under
   [source/slang/](../../../../source/slang). For a textual target,
   subclass `CLikeSourceEmitter` from
   [slang-emit-c-like.h](../../../../source/slang/slang-emit-c-like.h).
3. **Dispatcher.** Wire the backend into
   [slang-emit.cpp](../../../../source/slang/slang-emit.cpp) — both
   the `#include` and the dispatch logic in
   `emitEntryPointsSourceFromIR`.
4. **Prelude.** If the emitted code requires runtime support, add
   the prelude source under [prelude/](../../../../prelude) and
   register its embedded string for the new `SourceLanguage` in the
   `Session` constructor in
   [slang-global-session.cpp](../../../../source/slang/slang-global-session.cpp),
   alongside the CUDA, C++, and HLSL preludes.
   `emitEntryPointsSourceFromIR` in
   [slang-emit.cpp](../../../../source/slang/slang-emit.cpp) then
   writes that string into the generated source itself. Emit a
   `#include` only for a separate runtime header you intend to ship
   next to the generated output.
5. **Capability atoms.** Add atoms to
   [slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef)
   so the front-end can reject features the new target does not
   support.
6. **Target-specific IR passes.** If the target needs custom
   legalization, add `slang-ir-<target>-legalize.cpp` and gate it on
   the `TargetRequest`. See
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
7. **Profile entries.** If the new target supports versioned
   profiles, extend
   [slang-profile-defs.h](../../../../source/slang/slang-profile-defs.h)
   and the supporting tables in
   [slang-profile.cpp](../../../../source/slang/slang-profile.cpp).
8. **Tests.** Add fixtures under [tests/](../../../../tests),
   including HLSL/GLSL parity tests where applicable. See
   [CLAUDE.md](../../../../CLAUDE.md) for the test-directive
   conventions.
9. **User-guide updates.** A new target is a user-visible feature;
   add an entry under [docs/user-guide/](../../../user-guide).

## What is not in this document

- The full list of capability atoms — it lives in
  [slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef)
  and the auto-generated reference page.
- The detailed profile-version table — it lives in
  [slang-profile-defs.h](../../../../source/slang/slang-profile-defs.h).
- The user-facing target documentation — see
  [../../../user-guide/](../../../user-guide) and
  [../../../command-line-slangc-reference.md](../../../command-line-slangc-reference.md).
- Per-target emitter behavior — which extension a backend declares,
  which decoration it attaches, how a particular construct is
  spelled in the emitted language, and which command-line options
  change that spelling. Those belong to the matching page under
  [../target-pipelines/](../target-pipelines).
