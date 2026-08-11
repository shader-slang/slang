---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:48:00Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 8de686864f8c89a689087094669d66b19be061b10c489eb3d49177dc519b34b4
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Code Emission

This document describes how the legalized IR (output of
[05-ir-passes.md](05-ir-passes.md)) is turned into target code:
HLSL, GLSL, SPIR-V, Metal, WGSL, C++, CUDA, PyTorch glue, LLVM IR,
or VM bytecode. The intended reader is a developer adding or
modifying a target backend.

## Inputs and outputs

- **Input**: a linked, specialized, and target-lowered `IRModule`
  produced by `linkAndOptimizeIR`
  ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 970)
  for one `TargetRequest`. Note that direct SPIR-V is not fully
  legalized at this boundary: `linkAndOptimizeIR` returns first, and
  `emitSPIRVFromIR` then runs `legalizeIRForSPIRV` itself immediately
  before emission, so SPIR-V legalization is part of this step rather
  than the preceding one.
- **Output**: a target artefact wrapped in an `IArtifact`
  (declared in
  [slang-artifact.h](../../../../source/compiler-core/slang-artifact.h)).
  The artefact carries the textual or binary code and, when one was
  produced, an associated source-map artifact. Post-emit metadata is
  attached per emit path: the source and direct-SPIR-V artifacts
  associate `linkedIR.metadata`, while the HostVM and LLVM dispatch
  functions do not. Dependency-file output is *not* attached to the
  artefact: `writeDependencyFile` writes Make-style rules to the
  separately configured dependency-output path.

## Emit dispatcher

Dispatch happens at two levels.

The **outer** level picks the emit path for a `CodeGenTarget`. It lives
in [slang-code-gen.cpp](../../../../source/slang/slang-code-gen.cpp).
`CodeGenContext::emitEntryPoints` splits the targets into the textual
ones (`HLSL`, `GLSL`, `Metal`, `WGSL`, the C/C++/CUDA source and header
variants, `PyTorchCppBinding`), which go to `emitEntryPointsSource`, and
the binary / host ones, which go to `CodeGenContext::_emitEntryPoints`.
That second switch routes SPIR-V to `emitSPIRVForEntryPointsDirectly`
when `shouldEmitSPIRVDirectly()` is set, the CPU targets to
`emitLLVMForEntryPoints` when `isCPUTargetViaLLVM`,
`CodeGenTarget::HostVM` to `emitHostVMCode`, the `*Assembly` targets to
a recursive compile of the corresponding binary target followed by
downstream disassembly, and everything else to
`emitWithDownstreamForEntryPoints` — which itself emits source first,
via `emitEntryPointsSource`, and then hands it to DXC / FXC / glslang /
the Metal toolchain. The three direct entry points are defined in
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp) at lines 3500,
3587, and 3544 respectively; each calls `linkAndOptimizeIR` itself.

The **inner** level picks the source emitter for textual output.
`emitEntryPointsSource` either passes user source straight through (for
pass-through compilation) or calls
`CodeGenContext::emitEntryPointsSourceFromIR`
([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 2746 at
`source_commit`). That function:

1. Resolves `LineDirectiveMode` (GLSL targets default to
   `LineDirectiveMode::GLSL`; WGSL targets to `None`, because WGSL has
   no line directives) and constructs a `SourceWriter`
   ([slang-emit-source-writer.h](../../../../source/slang/slang-emit-source-writer.h))
   to buffer the emitted text, optionally with a `SourceMap`.
2. Selects the `CLikeSourceEmitter` subclass. `CodeGenTarget::PyTorchCppBinding`
   is special-cased to `TorchCppSourceEmitter`; every other target is
   mapped through `CLikeSourceEmitter::getSourceLanguage(target)` to a
   `SourceLanguage` and then to `CPPSourceEmitter`, `GLSLSourceEmitter`,
   `HLSLSourceEmitter`, `CUDASourceEmitter`, `MetalSourceEmitter`, or
   `WGSLSourceEmitter`. A target with no emitter fails with
   `Diagnostics::UnableToGenerateCodeForTarget`.
3. Runs `linkAndOptimizeIR` with the emitter attached to
   `LinkingAndOptimizationOptions` (the C / C++ / CUDA source languages
   clear `shouldLegalizeExistentialAndResourceTypes`), then
   `simplifyForEmit`, then `sourceEmitter->emitModule`, which walks the
   IR and emits one declaration / function at a time. Backends consult
   the precedence helper
   ([slang-emit-precedence.h](../../../../source/slang/slang-emit-precedence.h))
   as they print expressions; the result is precedence-aware but not
   minimal (see
   [Operator precedence and parenthesization](#operator-precedence-and-parenthesization)).
4. Stitches the final text: front matter (`emitFrontMatter`), the
   prelude, `emitPreModule`, then the module body.
5. Wraps the result in an `IArtifact`, attaches the post-emit metadata
   and (if one was produced) the source-map artifact, and hands it back.

The `#include`s at the top of
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp) pull in the
header-backed emit helpers used by this file (C-like subclasses, LLVM,
VM, Torch, Slang round-trip). Direct SPIR-V is not header-included here;
it is wired via the `emitSPIRVFromIR` forward declaration (line 2993)
and implemented in
[slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp).

## Backends

### HLSL

[slang-emit-hlsl.h](../../../../source/slang/slang-emit-hlsl.h) /
[slang-emit-hlsl.cpp](../../../../source/slang/slang-emit-hlsl.cpp).
Emits HLSL source text.
[slang-emit-hlsl-prelude.cpp](../../../../source/slang/slang-emit-hlsl-prelude.cpp)
holds the inline HLSL snippets `HLSLSourceEmitter` injects on demand
(the 64-bit cast helper, the cooperative-vector and cooperative-matrix
preludes) plus the name-mapping helpers described below; the shipped
per-language prelude is the separate header
[slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) under
[prelude/](../../../../prelude). HLSL output is typically handed off to
DXC or FXC via the downstream-compiler bridge in
[source/compiler-core/](../../../../source/compiler-core).

**Named constants are never emitted as raw integers.** Two mechanisms
in this backend implement that rule:

- Work-graph attributes read the symbolic value out of the IR
  decoration. `emitEntryPointAttributesImpl` emits
  `[NodeLaunch("...")]` by writing the `IRStringLit` payload of
  `IRNodeLaunchDecoration` verbatim, and emits `[NodeMaxDispatchGrid]`,
  `[NodeDispatchGrid]`, `[NodeID]`, and `[NodeIsProgramEntry]` from
  their own decorations. `emitSimpleFuncParamImpl` does the same for the
  parameter-level `[MaxRecords]`, `[NodeID]`, `[NodeArraySize]`, and
  `[AllowSparseNodes]`.
- Barrier flag sets are expanded back into DXC's flag tokens.
  `tryEmitInstExprImpl` handles `kIROp_GetEnumBarrierMemoryTypeFlags`
  and `kIROp_GetEnumBarrierSemanticFlags` by calling
  `emitNamedMemoryTypeFlagSet` / `emitNamedSemanticFlagSet`
  ([slang-emit-hlsl-prelude.cpp](../../../../source/slang/slang-emit-hlsl-prelude.cpp)
  lines 553 and 586), which validate the value and print
  `UAV_MEMORY`, `GROUP_SHARED_MEMORY`, `ALL_MEMORY`, `GROUP_SYNC`,
  `DEVICE_SCOPE`, and friends joined with `|`. `shouldFoldInstIntoUseSites`
  forces these getter ops to fold into their use site, because the flag
  expression has no standalone HLSL temporary form.

Work-graph record types (`DispatchNodeInputRecord`,
`GroupNodeOutputRecords`, `NodeOutput`, ...) are spelled by
`emitWorkGraphRecordType`, which maps the IR opcode to the HLSL type
name and re-emits the element type as a generic argument.

### GLSL

[slang-emit-glsl.h](../../../../source/slang/slang-emit-glsl.h) /
[slang-emit-glsl.cpp](../../../../source/slang/slang-emit-glsl.cpp).
Emits GLSL source text. GLSL output is consumed by
`source/slang-glslang/` to produce SPIR-V via Khronos `glslang` when
the user asks for SPIR-V via GLSL.

### SPIR-V (direct)

[slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp).
Emits SPIR-V binary directly without going through GLSL, from
`SPIRVEmitContext`. The opcode tables are split out into
[slang-emit-spirv-ops.h](../../../../source/slang/slang-emit-spirv-ops.h)
and
[slang-emit-spirv-ops-debug-info-ext.h](../../../../source/slang/slang-emit-spirv-ops-debug-info-ext.h).

The natively emitted blob is already a complete module, so
`createArtifactFromIR`
([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 3292)
decides up front whether any downstream (`slang-glslang` /
SPIRV-Tools) work is needed at all, and only loads that compiler when
it is. The four conditions are: an optimization level above
`OptimizationLevel::None` or explicit `-Xspirv-opt` arguments; more
than one SPIR-V module to link (embedded downstream IR from separate
compilation); SPIR-V validation enabled; or separate debug info
requested. A plain single-module `-O0 -target spirv` compile therefore
never touches `slang-glslang`. When the optimizer does run, the
`-Xspirv-opt` arguments are forwarded through
`downstreamOptions.compilerSpecificArguments` on top of the `-OX`
preset.

### Metal

[slang-emit-metal.h](../../../../source/slang/slang-emit-metal.h) /
[slang-emit-metal.cpp](../../../../source/slang/slang-emit-metal.cpp).
Emits Metal Shading Language. The inline builtin snippets it injects
through `ensurePrelude` (matrix `fmod`, matrix / vector reshape,
`simdgroup_matrix` ops, logging) are the
`kMetalBuiltinPrelude*` strings in
[slang-emit-metal-prelude.cpp](../../../../source/slang/slang-emit-metal-prelude.cpp).
`MetalSourceEmitter` does not create its `MetalExtensionTracker`; it
retains a reference (a `RefPtr`) to the one already created on the
`CodeGenContext`, so requirements it
records — the minimum Metal language version via
`requireMetalLanguageVersion`, and `requireLogging` for the `os_log`
path used by `kIROp_Printf` — survive past emission for the
downstream Metal compile.

### WGSL

[slang-emit-wgsl.h](../../../../source/slang/slang-emit-wgsl.h) /
[slang-emit-wgsl.cpp](../../../../source/slang/slang-emit-wgsl.cpp).
Emits WGSL. It is the one backend that overrides both
`supportsSwitchFallThrough` and `shouldEmitSwitchCaseTerminatingBreak`
to `false`: WGSL `case` bodies never fall through, so the `break` Slang
places at the tail of every case is redundant, and older `naga`
validators reject a `break` outside a loop. Only that trailing break is
dropped; early breaks inside a case are still emitted.

### C++

[slang-emit-cpp.h](../../../../source/slang/slang-emit-cpp.h) /
[slang-emit-cpp.cpp](../../../../source/slang/slang-emit-cpp.cpp).
Emits C++ source, paired with the preludes
[slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h),
[slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h),
[slang-cpp-types.h](../../../../prelude/slang-cpp-types.h),
[slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h),
and
[slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h).
The runtime support that emitted C++ links against lives in
[source/slang-rt/](../../../../source/slang-rt).

### CUDA

[slang-emit-cuda.h](../../../../source/slang/slang-emit-cuda.h) /
[slang-emit-cuda.cpp](../../../../source/slang/slang-emit-cuda.cpp).
Emits CUDA source. Prelude:
[slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h).

### Torch

[slang-emit-torch.h](../../../../source/slang/slang-emit-torch.h) /
[slang-emit-torch.cpp](../../../../source/slang/slang-emit-torch.cpp).
Emits the PyTorch C++ glue used to bind Slang shaders into PyTorch
extensions. Prelude:
[slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h).

### LLVM

[slang-emit-llvm.h](../../../../source/slang/slang-emit-llvm.h) /
[slang-emit-llvm.cpp](../../../../source/slang/slang-emit-llvm.cpp).
Hands off to the JIT / native compilation path implemented in
[source/slang-llvm/](../../../../source/slang-llvm). Which artefact it
produces depends on the target: textual LLVM assembly for `HostLLVMIR` /
`ShaderLLVMIR` (`emitLLVMAssemblyFromIR`), object code for
`HostObjectCode` / `ShaderObjectCode` (`emitLLVMObjectFromIR`), and a
JIT library for `HostHostCallable` / `ShaderHostCallable`
(`emitLLVMJITFromIR`).
Helper header: [slang-llvm.h](../../../../prelude/slang-llvm.h).

### VM

[slang-emit-vm.h](../../../../source/slang/slang-emit-vm.h) /
[slang-emit-vm.cpp](../../../../source/slang/slang-emit-vm.cpp).
`emitVMByteCodeForEntryPoints` builds a `VMByteCodeBuilder` from the
linked IR — Slang's interpreter bytecode, used by the `slangi` tool and
by `INTERPRET` tests, see [CLAUDE.md](../../../../CLAUDE.md). The
interpreter runs on the CPU and has no notion of global shader
parameters, so `ByteCodeEmitter` detects any function that references
an `IRGlobalParam` (directly, or through a global-scope instruction such
as a load of it) and reports
`Diagnostics::GlobalParamNotSupportedByInterpreter` once per parameter;
`emitVMByteCodeForEntryPoints` then returns `SLANG_FAIL` rather than
handing back malformed bytecode.

### Slang round-trip

[slang-emit-slang.h](../../../../source/slang/slang-emit-slang.h) /
[slang-emit-slang.cpp](../../../../source/slang/slang-emit-slang.cpp)
declare `emitSlangDeclarationsForEntryPoints`, the hook for re-emitting
Slang declarations from IR. At `source_commit` the implementation is a
stub that produces an empty string. Its one caller is `emitHostVMCode`,
which compiles the returned declarations into the `kernel` module that
is serialized alongside the VM bytecode.

### Shared C-like base

The textual backends (HLSL, GLSL, Metal, WGSL, C++, CUDA, Torch)
share most of their machinery:

- [slang-emit-c-like.h](../../../../source/slang/slang-emit-c-like.h) /
  [slang-emit-c-like.cpp](../../../../source/slang/slang-emit-c-like.cpp)
  — the base class `CLikeSourceEmitter`. It walks the IR,
  declares a virtual interface (`emitDeclarator`, `emitType`,
  `emitOperand`, ...) for the per-target subclasses to override, and
  implements all the parts that the targets share.
- [slang-emit-base.h](../../../../source/slang/slang-emit-base.h) /
  [slang-emit-base.cpp](../../../../source/slang/slang-emit-base.cpp)
  — `SourceEmitterBase`, the lowest common interface. Besides
  `CLikeSourceEmitter` it is also the base of `SPIRVEmitContext` in
  [slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp)
  and of the SPIR-V legalization context, so the shared helpers
  (`getSpecializedValue`, `getVarLayout`, `extractBaseType`,
  `handleRequiredCapabilities`) are available on both paths. The LLVM
  and VM paths do not use it.

The class hierarchy is not flat: `CUDASourceEmitter` and
`TorchCppSourceEmitter` both derive from `CPPSourceEmitter` rather than
from `CLikeSourceEmitter` directly. A new textual backend
typically subclasses `CLikeSourceEmitter` and overrides only the
operations that differ from the C-like default. Divergences that are
purely syntactic go through small virtual predicates rather than
target checks in shared code — for example `supportsSwitchFallThrough`,
`shouldEmitSwitchCaseTerminatingBreak` (both overridden by WGSL),
`shouldFoldInstIntoUseSites`, and `emitTempModifiers` (overridden by
C++, Metal, and WGSL; the latter two diagnose
`Diagnostics::PreciseQualifierUnsupportedOnTarget`).

## Source-writer abstraction

[slang-emit-source-writer.h](../../../../source/slang/slang-emit-source-writer.h)
declares `SourceWriter`, the buffer that all textual backends write
into. Its features (visible in the header):

- `emit*` overloads for raw text, integers, doubles, and `Name`s.
- `indent()` / `dedent()` to manage indentation.
- `advanceToSourceLocation(SourceLoc)` to emit `#line` (or GLSL
  `#line`-equivalent) directives so that downstream compilers report
  errors at the user's original source position.
- `LineDirectiveMode` configures the directive style (C / GLSL /
  none).
- A `SourceMap` companion for source-mapping debug information.

## Operator precedence and parenthesization

[slang-emit-precedence.h](../../../../source/slang/slang-emit-precedence.h)
/
[slang-emit-precedence.cpp](../../../../source/slang/slang-emit-precedence.cpp)
encode operator precedences for the textual targets, so emitted code is
precedence-aware rather than minimally parenthesized. `maybeEmitParens`
starts from the precedence comparison, then deliberately forces extra
parentheses around bitwise, logical, relational, equality, and shift
combinations — even where the language's own precedence already
preserves semantics — because downstream compilers warn about those
combinations. Each backend asks the precedence helper before printing a
binary or unary operator.

## Preludes

The targets listed in the table below ship a prelude header in
[prelude/](../../../../prelude) that the emitted code includes; the
other textual targets rely on vocabulary their own backend emits. A
prelude defines target-specific built-in functions, type aliases,
and helper macros so that the emitted source is self-contained.

`emitEntryPointsSourceFromIR` chooses which one to emit right after the
front matter: `CodeGenTarget::PyTorchCppBinding` gets
`get_slang_torch_prelude()`, a heterogeneous target
(`isHeterogeneousTarget`) gets `get_slang_cpp_host_prelude()`, and
everything else gets `session->getPreludeForLanguage(sourceLanguage)`,
which returns the prelude string registered on the global session for
that `SourceLanguage`. This is separate from the inline snippets a
backend injects mid-emission through `ensurePrelude` (the
`kMetalBuiltinPrelude*` and HLSL cooperative-vector / cooperative-matrix
strings), which are compiled into the emitters rather than shipped as
headers.

| Target | Prelude header |
| --- | --- |
| HLSL | [slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) |
| CUDA | [slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h) |
| C++ shader | [slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h), [slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h), [slang-cpp-types.h](../../../../prelude/slang-cpp-types.h), [slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h) |
| C++ host | [slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h) |
| Torch | [slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h) |
| LLVM | [slang-llvm.h](../../../../prelude/slang-llvm.h) |

GLSL, Metal, WGSL, and SPIR-V have no `prelude/` header; the built-in
vocabulary they rely on is emitted from their own backend files, as with
the `kMetalBuiltinPrelude*` strings in
[slang-emit-metal-prelude.cpp](../../../../source/slang/slang-emit-metal-prelude.cpp).
The preludes are introduced from the core-module side in
[../cross-cutting/core-module.md](../cross-cutting/core-module.md).

## Dependency-file output

[slang-emit-dependency-file.h](../../../../source/slang/slang-emit-dependency-file.h)
/
[slang-emit-dependency-file.cpp](../../../../source/slang/slang-emit-dependency-file.cpp)
produces Make-style dependency files. `writeDependencyFile` writes one
`<output-file>: <dep> <dep> ...` statement per compile product (using
`-` as the target when output goes to stdout), escaping paths as make
requires. It is a no-op unless an output path was requested. Used by
build systems to track header / module dependencies.

## Adding a new backend

1. Add `slang-emit-<target>.{h,cpp}` under
   [source/slang/](../../../../source/slang). For a textual target,
   subclass `CLikeSourceEmitter`.
2. Register the new backend in both dispatchers. For a textual target,
   add the arm in `emitEntryPointsSourceFromIR`
   ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp)) — the
   `switch (target)` / `switch (sourceLanguage)` block that constructs
   the `CLikeSourceEmitter` subclass — plus the `#include` for the
   header, and list the new `CodeGenTarget` under the source-target arm
   of `CodeGenContext::emitEntryPoints` in
   [slang-code-gen.cpp](../../../../source/slang/slang-code-gen.cpp).
   A direct/non-textual backend instead follows the pattern of SPIR-V,
   LLVM, and VM bytecode: a separate emit function
   (`emitSPIRVForEntryPointsDirectly`, `emitLLVMForEntryPoints`,
   `emitVMByteCodeForEntryPoints`) that calls `linkAndOptimizeIR`
   itself and is invoked from `CodeGenContext::_emitEntryPoints`.
3. Add a prelude under [prelude/](../../../../prelude) if the emitted
   code needs runtime support, then expose its generated string and
   register it for the relevant `SourceLanguage` (as
   `slang-global-session.cpp` does for CUDA, C++, and HLSL) so that
   `emitEntryPointsSourceFromIR` writes the text into the output. Emit a
   `#include` only for a runtime header you deliberately ship and deploy
   alongside the generated source.
4. Add capability bits in
   [slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef)
   so the front-end can reject features the new target does not
   support — see
   [../cross-cutting/targets.md](../cross-cutting/targets.md).
5. Add target-specific IR legalization passes if needed
   ([05-ir-passes.md](05-ir-passes.md)) and gate them on the
   `TargetRequest`.
6. Add tests under [tests/](../../../../tests), typically using
   `COMPARE_COMPUTE` or `INTERPRET` directives plus per-backend
   variants — see [CLAUDE.md](../../../../CLAUDE.md) for the test
   conventions.

## Paths outside the watched set

This page's manifest entry watches `source/slang/slang-emit.cpp`,
`source/slang/slang-emit-*.{h,cpp}`,
`source/slang/slang-code-gen.cpp`, and
`source/slang/slang-global-session.cpp`. One fact above still comes
from outside that set, so drift in it will not mark this page stale:

- The prelude header contents themselves live under
  [prelude/](../../../../prelude); only their registration on the
  global session is watched. Adding `prelude/*.h` to `watched_paths`
  would cover the prelude table.
