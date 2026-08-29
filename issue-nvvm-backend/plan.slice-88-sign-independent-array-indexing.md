# Admit sign-independent pointer indexing and shared-memory suite coverage

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM accepts canonical signed or unsigned 32-bit SSA and literal
indices for every already-supported pointer and fixed-array relation. The existing shared-memory
shaders
`tests/compute/groupshared.slang`,
`tests/language-feature/execution-model/groupshared-barrier-functional.slang`, and
`tests/language-feature/execution-model/groupshared-multi-barrier-functional.slang` must execute
through direct libNVVM with their established CUDA results. Their direct PTX must expose shared
storage, shared loads/stores, and synchronization, and CUDA 12.9 `ptxas -arch=sm_70` must accept
each module.

## Progress

- [x] (2026-08-29) Completed and committed Slice 87 as `3783bbb80` with 358/358 NVVM tests.
- [x] (2026-08-29) Probed selected vector, structured-buffer, matrix, and shared-memory suite
  candidates to find the next exact canonical boundary.
- [x] (2026-08-29) Confirmed that `groupshared.slang` already compiles to direct PTX and that both
  functional shared-memory shaders stop only at an unsigned fixed-array index.
- [x] (2026-08-29) Generalized fixed-array indices and unsigned literal pointer indices through
  the existing sign-independent 32-bit integer transport without changing pointee/storage policy.
- [x] (2026-08-29) Strengthened focused fake-boundary coverage for unsigned shared-array indexing
  and retained adjacent unsupported shared shapes.
- [x] (2026-08-29) Registered the three existing shared-memory shaders for direct runtime and PTX
  evidence and ran CUDA 12.9 `ptxas`.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable
  documents, self-reviewed, and prepared this plan with the implementation for commit.

## Surprises and Discoveries

- `tests/compute/groupshared.slang` already crosses the direct backend successfully. Its final PTX
  contains a demoted 16-byte shared array, `st.shared.u32`, `ld.shared.u32`, and `bar.sync 0`, but
  the ordinary suite has no direct lane proving those semantics.
- The functional single-barrier shader's final graph contains two accesses to the same canonical
  `groupshared int[4]`. The neighbor access uses signed `neighborIdx`; the initial access uses the
  canonical unsigned dispatch/group index. The latter alone triggers E52017 `signed i32 value`.
- Raw-buffer indexing and ordinary scalar pointer offsets already use `_validateInteger32Value`,
  and the real provider's generic array GEP accepts any LLVM integer index. The fixed-array branch
  is the remaining sign-specific validation island.
- The first focused run proved that `_validateInteger32Value`'s historical UInt-SSA-only policy
  moved the former unsigned literal pointer/array controls to E52017 `integer_constant`. Selected
  UInt32 literals are already canonical executable constants, so keeping that split would make
  index acceptance depend on whether optimization folded an otherwise identical value.
- Vector/helper candidates stop at genuinely different contracts (`helper function parameter`,
  dynamic vector extraction, vector resource storage, or matrix construction). They do not belong
  in this shared-memory/indexing slice.
- The suite runner adds `-O0` to SIMPLE compile tests. That keeps a single physical barrier helper
  plus call sites, while standalone optimized compilation inlines each `bar.sync`. PTX checks must
  therefore prove the retained call topology at `-O0`; optimized artifacts separately prove final
  instruction counts.
- A whole-file `groupshared.slang` run also synthesized an unrelated WebGPU lane that failed in
  Dawn bind-group validation on this machine. The original CUDA/Vulkan lanes passed, and the six
  explicitly selected new direct-NVVM lanes passed independently.

## Decision Log

- Decision: use one sign-independent 32-bit integer contract, including exact literals, for all
  admitted pointer and array indices.
  Rationale: signedness does not change the physical LLVM i32 index or the source pointer relation.
  The producer already provides a canonical UInt execution index, so converting it merely to
  satisfy the consumer would create a redundant representation.
  Date/author: 2026-08-29, Codex.
- Decision: register three existing shared-memory tests rather than adding a new file-backed
  fixture.
  Rationale: these shaders already have asymmetric runtime expectations and exercise one barrier,
  multiple barriers, helper calls, shared loads/stores, and conventional global resources. They
  are stronger compatibility evidence than a duplicate backend-only source.
  Date/author: 2026-08-29, Codex.
- Decision: retain the exact signed-i32 shared element/storage family.
  Rationale: this slice changes index transport only. UInt/Float/vector shared storage,
  group-shared atomics, nested arrays, dynamic shared memory, and pointer escape are independent
  physical and semantic contracts with existing negative boundaries.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The direct consumer now has one sign-independent i32 index contract. Exact UInt32 literals and
available UInt32 SSA values reach ordinary pointer offsets, raw-buffer element offsets, device
fixed-array GEPs, and shared fixed-array GEPs without an inserted cast. The real/fake providers
remain unchanged because their physical integer handles were already signless and generic. The
former UInt-literal pointer/array unsupported controls now pass through a parameterized positive
test, while adjacent element, aggregate, helper, local-memory, and atomic boundaries remain.

The two new focused units pass with the established shared and unsupported controls, 4/4. The
three existing shaders pass the complete CUDA-scoped regression 10/10, including all six new direct
runtime/PTX lanes. Their outputs are respectively `1, 0, 3, 2`, `10, 20, 30, 0`, and
`2, 3, 0, 1`. Optimized direct PTX sizes are 933, 1,315, and 1,404 bytes. CUDA 12.9.86 `ptxas`
accepts all three for `sm_70`, producing cubins of 3,040, 3,168, and 3,168 bytes. The standalone
provider and Release `slang-unit-test`, `slangc`, and `slang-test` targets build, and the complete
NVVM prefix passes 360/360.

The final self-review inventory contains two production generalizations and no fallback. First,
`_validateInteger32Value` accepts the exact UInt32 executable-constant representation already
owned by the selected-integer constant producer; it does not reinterpret other widths or recover
syntax. Second, the fixed-array relation uses that same validator after its base/result element,
address-space, access, and array-shape checks succeed. The motivating UInt execution index is the
canonical final value produced by CUDA varying legalization, and a cast would create an alternate
representation solely for this consumer. The fake additions observe exact parameter/constant
producers and generic pointer consumers. No graph walk, source-name match, bounds assumption,
`inbounds` claim, provider special case, ABI change, or text rewrite was introduced.

Final commands and observed evidence:

- `cmake --build build --config Release --target slang-unit-test slangc slang-test` built all host
  targets after the implementation changes.
- `cmake --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target
  slang-llvm-nvvm` built the standalone provider.
- `build\Release\bin\slang-test.exe
  slang-unit-test-tool/nvvmSlangUnsignedSharedArrayIndexUsesDirectPipeline
  slang-unit-test-tool/nvvmSlangUnsignedConstantPointerIndicesUseDirectPipeline
  slang-unit-test-tool/nvvmSlangSharedMemoryUsesDirectPipeline
  slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` passed 4/4.
- The six explicitly selected direct runtime/PTX lanes passed 6/6, and the CUDA-scoped three-file
  run passed 10/10 with six non-CUDA lanes ignored.
- `build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm` passed 360/360.
- CUDA 12.9.86 `ptxas.exe -arch=sm_70` accepted `groupshared.ptx`,
  `barrier-functional.ptx`, and `multi-barrier.ptx`.
- Pinned clang-format 17 and `git diff --check` passed.

## Context and Current Pipeline

Consider the existing source:

```slang
groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    sharedData[idx] = int(idx) * 10;
    GroupMemoryBarrierWithGroupSync();
    int neighborIdx = (int(idx) + 1) % 4;
    outputBuffer[idx] = sharedData[neighborIdx];
}
```

CUDA varying legalization produces the canonical unsigned execution index. Final linked IR uses
that UInt directly in `getElementPtr(sharedData, idx)`, while the neighbor calculation produces an
Int index for the second access. `_validateNVVMFunction` recognizes both exact fixed-array pointer
relations but calls `_validateI32Value` for their indices, so it rejects the first access before
provider discovery. The raw-buffer and scalar-pointer branches beside it already call
`_validateInteger32Value`.

Emission lowers both accepted relations through the facade's existing
`emitArrayElementPointer(module, baseArrayPointer, elementIndex, outPointer)`. The LLVM provider
checks that the index has integer type, then creates a non-inbounds two-index GEP. LLVM integer
types carry no signedness, and the provider needs no API or implementation change.

## Scope and Non-Goals

In scope are signed/unsigned i32 SSA and literal pointer/fixed-array indices, the focused fake
shared-memory trace, provider-independent negative preservation, three named existing
shared-memory runtime/PTX lanes, and `ptxas` validation.

Out of scope are additional index widths, floating indices, bounds repair, inbounds provenance,
array values, local arrays, new device/shared array element types, group-shared atomics, nested or
dynamic shared storage, pointer escape, helper/vector ABI expansion, and matrix support.

## Architecture and Invariants

The producer's exact canonical index type remains the source of truth. The direct consumer accepts
only an available or exact executable-literal signed/unsigned 32-bit integer value; it neither
inserts a cast nor infers a different source spelling. Array base/result pointer checks continue
to prove identical element type, address space, access qualifier where applicable, and an
already-supported fixed-array shape.

The provider receives the same physical i32 handle for signed and unsigned semantic inputs. It
creates a non-inbounds GEP because a Slang subscript does not itself prove LLVM's stronger
provenance contract. Shared storage remains one nonempty fixed Int array in address space 3.

## Interfaces and Dependencies

Change only direct-emitter validation and focused tests. No builder callback, operation ID,
semantic descriptor, feature constant, structure-size field, compatibility branch, ABI revision,
or provider implementation changes.

The acceptance environment is the existing standalone LLVM provider, CUDA 12.9 libNVVM, a CUDA
GPU for comparison lanes, and CUDA 12.9 `ptxas` targeting `sm_70`.

## Milestones

1. Replace the fixed-array branch's signed-i32 validator with the existing sign-independent i32
   transport validator and admit exact UInt32 executable literals consistently.
2. Make the focused shared-memory fixture carry canonical unsigned indices into its array GEPs,
   move the former unsigned literal pointer/array controls to positive coverage, and assert their
   fake producer-consumer identity.
3. Add direct runtime and PTX directives/checks to the three named existing shaders.
4. Compile and inspect each direct module, run the existing expected-result comparisons, and run
   CUDA 12.9 `ptxas -arch=sm_70`.
5. Run focused/adjacent units and the complete NVVM prefix, update the design and capability
   ledger, perform the input-shape audit, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- the focused fake trace observes both signed and unsigned i32 shared-array indices through the
  same generic array-element operation;
- Float shared arrays, local arrays, helper-array pointers, UInt shared storage/atomics, and other
  adjacent unsupported controls still stop before provider discovery;
- all three existing shaders pass their established CUDA lane and new direct-libNVVM runtime lane;
- direct PTX exposes address-space-3 storage, shared loads/stores, and the expected number of
  barriers for each source;
- CUDA 12.9 `ptxas -arch=sm_70` accepts all three direct modules;
- standalone provider and Release `slang-unit-test`, `slang-test`, and `slangc` targets build;
- focused unit/file tests and the complete NVVM prefix pass; and
- pinned clang-format 17 and `git diff --check` pass.

Record exact commands and observed counts in this plan before commit.

## Failure and Recovery

The validator edit is independently reversible and changes no serialized interface. If a runtime
result differs, compare final linked IR, direct PTX, and CUDA/NVRTC PTX for index width, shared
symbol size/alignment, and barrier count before changing the contract. If libNVVM or `ptxas`
rejects an unsigned-index module, preserve the measured failure and restore E52017; do not insert a
source-semantic cast or rewrite text merely to pass the provider.

## Artifacts and Hand-Off

Keep final linked IR, direct PTX, CUDA runtime output, and `ptxas` cubins under ignored
`build/nvvm-slice88/`. Distill stable sign-independent array-index and shared-memory corpus status
into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`, then include this completed plan in Slice 88 as
explicitly requested.
