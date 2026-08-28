# Add flat constant buffers and explicit immutable loads

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM route accepts nonempty flat selected-scalar
`ConstantBuffer<T>` globals as the second member of the same structural parameter-group family as
the established `ParameterBlock<T>` support. Scalar member reads carry an explicit immutable-load
semantic through the generic builder interface and LLVM lowers them to PTX `ld.global.nc`.

The existing `tests/cuda/constant-buffer-ldg.slang` gains a direct PTX lane that checks the
non-coherent read, while `tests/cuda/wave-lane-index-multidim-3d.slang` gains a direct runtime lane.
The probe of `tests/cuda/noinline.slang` is retained as evidence for a semantic function-attribute
gap, not registered as a false positive.

## Progress

- [x] (2026-08-28) Probed representative CUDA corpus files through the direct route and recorded
  the next structural failures plus the already-supported 3D wave case.
- [x] (2026-08-28) Captured final linked IR and CUDA/NVRTC PTX for
  `constant-buffer-ldg.slang`.
- [x] (2026-08-28) Verified from LLVM's NVPTX tests that `!invariant.load` on a global-address-space
  load lowers to `ld.global.nc`, without requiring an NVVM-specific intrinsic.
- [x] (2026-08-28) Generalized the flat scalar parameter-block classifier/lowering to an exact parameter-group
  classifier shared by `ParameterBlock<T>` and `ConstantBuffer<T>`.
- [x] (2026-08-28) Added an extensible load-flags argument to exact ABI revision 4 and made the LLVM provider
  attach invariant-load metadata when requested.
- [x] (2026-08-28) Reused the canonical immutable-location predicate for every direct load, with
  parameter-group positive, nested-group negative, and ordinary mutable-load fake coverage.
- [x] (2026-08-28) Registered the two supported corpus lanes, inspected and assembled direct PTX, ran
  focused and broad tests, updated durable documents, completed self-review, and committed the
  completed slice.

## Surprises and Discoveries

- The final direct IR for `constant-buffer-ldg.slang` contains no `CUDALDG` instruction. Direct
  preparation intentionally skips CUDA-source immutable-load lowering and leaves a normal
  `field_addr` plus `load` for the LLVM builder to annotate.
- CUDA/NVRTC represents the conventional global field as an eight-byte pointer in constant
  storage and emits `ld.const.u64` for that pointer followed by `ld.global.nc.u32` for the scalar
  member.
- LLVM's `llvm/test/CodeGen/NVPTX/ldg-invariant.ll` states and tests the required contract directly:
  an addrspace(1) load with `!invariant.load` becomes `ld.global.nc`, while the same load without
  metadata remains an ordinary `ld.global`.
- The corpus probe initially found that `noinline.slang` and the 3D multidimensional wave shader
  both compile through direct NVVM. Deeper review confirmed only the wave shader's semantics:
  direct emission ignores `IRNoInlineDecoration`, so the retained helper in unoptimized PTX is not
  evidence for the no-inline contract and that test must remain unregistered.
- Self-review found that `isPointerToImmutableLocation(getRootAddr(ptr))` is already the semantic
  source of truth used by CUDA-source lowering. Reusing it removed the initial parameter-group-only
  marker and automatically retains the OptiX SBT exception and future immutable pointer kinds.
- The first builder-prefix run passed 84 tests and failed only the version-string assertion that
  intentionally named ABI revision 3. Updating that exact expected identity to revision 4 produced
  85/85; no provider construction or serialization contract failed.

## Decision Log

- Decision: express immutable reads as a load flag on the existing structural `emitLoad` callback,
  not as a new `emitImmutableLoad` callback or a typed NVVM `ldg` intrinsic.
  Rationale: invariance is a property of a memory access, LLVM already owns the target lowering,
  and one extensible load-flags parameter keeps the provider interface generic as more memory
  semantics arrive. Revisit only if libNVVM rejects the metadata or generated PTX loses `.nc`.
  Date/author: 2026-08-28, Codex.
- Decision: treat exact flat scalar `ParameterBlock<T>` and `ConstantBuffer<T>` values as one
  parameter-group representation: a global pointer to the unpacked scalar element struct.
  Rationale: their measured direct IR shape and CUDA ABI are identical at this boundary; separate
  classifiers and lowering paths would duplicate the same structural contract. Revisit when a
  group kind has a genuinely different executable representation.
  Date/author: 2026-08-28, Codex.
- Decision: select the invariant flag with the existing
  `isPointerToImmutableLocation(getRootAddr(ptr))` predicate for every accepted direct load.
  Rationale: this is the canonical producer-side policy already used by CUDA-source lowering. It
  covers both group kinds and the compiler-synthesized outer constant block, preserves the OptiX
  SBT exclusion, and leaves mutable device loads untagged without duplicating opcode knowledge.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Flat selected-scalar constant buffers now compose through the same generic struct, pointer, keyed
field, and load operations as parameter blocks. The fake provider observes the exact outer
group-pointer and inner scalar graph, all constant-rooted loads carry the invariant flag, ordinary
device-pointer loads carry none, and a nested constant-buffer element stops before provider
discovery.

Builder ABI revision 4 adds one flags word to the existing load callback. The real provider emits
`!invariant.load`, the LLVM 7-compatible text reaches libNVVM, and the constant-buffer shader emits
`ld.const.u64` plus `ld.global.nc.u32`. CUDA 12.9 `ptxas` accepts that module and the new 3D-wave
direct module for `sm_70`.

The Release host build and isolated provider build pass. Builder tests pass 85/85, the conventional
parameter-group tests pass 4/4, the negative boundary passes, the complete NVVM prefix passes
340/340, `constant-buffer-ldg` passes 2/2, and the 3D wave comparison passes 3/3 across CUDA/NVRTC,
direct libNVVM, and Vulkan.

Self-review inventory:

- `asNVVMSupportedScalarParameterGroupType` survives because it replaces the narrower classifier
  with one exact opcode and flat-element boundary shared by type roles, field admission, and
  lowering; it does not create an alternative representation.
- `SlangNVVMLoadFlags` survives because invariance is one generic property on the existing memory
  operation, unknown bits reject, and LLVM metadata is the target-independent provider contract.
- The initial `isParameterGroupMember` field-address marker was removed. The shared
  `isPointerToImmutableLocation` predicate already owns this classification and carries the SBT
  exception; keeping another classifier would have duplicated producer semantics.
- The constant-buffer positive and nested negative survive because their final IR uses the same
  exact outer/inner keyed graph as the established parameter block. No name, byte offset,
  structural equivalence relation, fallback, syntax reconstruction, or aggregate flattening was
  added.

## Context and Current Pipeline

Consider the existing test:

```slang
struct Params { uint value; };
ConstantBuffer<Params> gParams;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(RWStructuredBuffer<uint> output, uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = gParams.value + tid.x;
}
```

CUDA global collection creates a conventional parameter struct containing the
`ConstantBuffer<Params>` field. Final linked IR forms `field_addr(globalParams, gParams)`, loads the
group value, forms `field_addr(groupValue, value)`, and loads the `uint`. Existing direct lowering
already handles the identical outer/inner graph for `ParameterBlock<Params>` except that its type
classifier accepts only `IRParameterBlockType`.

`source/slang/slang-emit.cpp` deliberately calls `lowerImmutableBufferLoadForCUDA` only for CUDA
source emission. Direct NVVM retains the ordinary IR load so `source/slang/slang-emit-nvvm.cpp`
can select an immutable memory semantic while `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` maps that
semantic to LLVM metadata. LLVM NVPTX then selects the read-only cache instruction.

## Scope and Non-Goals

In scope are exact `IRParameterBlockType` and `IRConstantBufferType` values whose nonempty element
struct contains only selected integer or float32 fields; their common pointer representation;
keyed scalar field addresses; explicit ordinary/invariant load flags; LLVM invariant-load metadata;
the existing constant-buffer static test; and adjacent already-supported corpus registrations.

Out of scope are nested structs, arrays, matrices, resources within groups, Boolean/half/double
members, aggregate group values, stores through group pointers, texture support, helper function
parameters, raw entry-point parameter expansion, and other LLVM memory metadata. These remain
deterministic pre-provider failures.

## Architecture and Invariants

The shared CUDA collector and layout remain the source of truth for outer field order and ABI.
The direct emitter recognizes a parameter group only when its opcode is exactly parameter block or
constant buffer, its element is an exact nonempty struct, and every field is an established scalar.
The provider representation is a global-address-space pointer to the generic lowered element
struct. The canonical Slang types remain distinct in the type map even when they reuse the same
provider pointer representation.

The existing keyed field resolver remains the only source of field-position truth. Load semantics
come from `isPointerToImmutableLocation(getRootAddr(ptr))`, the shared producer-side predicate used
by CUDA-source lowering. Both the outer conventional constant block and inner group storage are
immutable; mutable device-pointer reads remain ordinary. The rule does not inspect source field
names or reconstruct syntax.

The builder ABI accepts only declared flag bits. The real provider emits the same aligned,
non-volatile LLVM load as before and attaches empty `LLVMContext::MD_invariant_load` metadata only
when the invariant bit is present. This is semantic metadata, not a CUDA-specific instruction in
the construction API.

## Interfaces and Dependencies

Change `source/compiler-core/slang-nvvm-ir-builder-api.h` by adding `SlangNVVMLoadFlags`, none and
invariant constants, the flag argument on `SlangNVVMBuilderConstructionAPI::emitLoad`, and a new
exact ABI revision. Update the C++ wrapper, fake provider, real LLVM provider, ABI probes, and all
builder call sites together; no backward-compatibility adapter is retained.

Generalize the direct classifier and lowerer in
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}`, then use it in validation and emission in
`source/slang/slang-emit-nvvm.cpp`. LLVM 14's `LoadInst::setMetadata` and
`LLVMContext::MD_invariant_load` provide the implementation contract. LibNVVM continues to receive
the established LLVM 7-era textual dialect after serialization.

## Milestones

1. Revise the exact construction ABI and wrapper, validate flag bits in the real and fake
   providers, and add builder contract coverage for ordinary, invariant, and invalid loads.
2. Replace parameter-block-only classification/lowering with one exact flat parameter-group path
   shared by parameter blocks and constant buffers.
3. Carry the canonical immutable-location classification to `emitLoad`, while proving mutable
   device reads remain ordinary and address-space-1 immutable reads select `.nc`.
4. Add fake direct-emitter positive coverage for a conventional constant buffer and retain a
   nested-group negative that stops before provider discovery.
5. Register the constant-buffer and 3D wave corpus lanes, retain no-inline as an unsupported
   semantic gap, inspect LLVM assembly/PTX, assemble the direct PTX, run focused and broad tests,
   update documentation, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- builder unit coverage observes `!invariant.load` on requested reads, no metadata on ordinary
  reads, and `SLANG_E_INVALID_ARG` for unknown flag bits;
- fake direct emission records invariant constant-rooted loads for both parameter block and
  constant buffer, while ordinary mutable device-pointer reads stay untagged;
- a nested or otherwise unsupported constant-buffer element stops with E52017 before builder
  discovery;
- `constant-buffer-ldg.slang` direct PTX contains `ld.const.u64` and `ld.global.nc.u32`;
- `noinline.slang` remains unregistered until direct emission preserves its function attribute;
- the 3D wave shader passes CUDA/NVRTC and direct libNVVM runtime lanes;
- CUDA 12.9 `ptxas -arch=sm_70` accepts each new direct module;
- the Release host build, standalone provider build, and complete NVVM prefix pass;
- clang-format, `git diff --check`, and repository status checks pass; and
- `external/slang-binaries/` and generated build artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects invariant metadata, retain the generic flag but isolate whether the LLVM 7 text
rewriter drops or misspells the metadata before considering an intrinsic. If LLVM assembly contains
the metadata but PTX lacks `.nc`, compare the address space and optimization pipeline against
`llvm/test/CodeGen/NVPTX/ldg-invariant.ll`; do not special-case a PTX mnemonic in the emitter.

If constant-buffer IR differs from the measured field-address graph after another pass, stop at
preflight and inspect the producer. Do not accept nested groups by walking arbitrary operand graphs
or flattening source syntax. All changes are localized to the exact builder ABI, structural
classification, and direct emitter and can be reverted as one slice.

## Artifacts and Hand-Off

Keep linked-IR dumps, LLVM assembly, generated CUDA, direct PTX, `ptxas` output, and focused test
logs under ignored `build/` paths. Distill the common parameter-group representation, load-flags
contract, immutable-load evidence, registered corpus cases, no-inline gap, remaining corpus stop,
and validation results into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`. Complete progress, outcomes, surprises, and the
self-review inventory before committing this plan with Slice 78.
