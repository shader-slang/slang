# Lower signed device-pointer offsets through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 10 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical signed-`i32` element offsets on the raw
device `Ptr<int>` ABI established by Slice 6. The shortest observable source is:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int index)
{
    *(destination + index) = *(source + index);
}
```

The final linked Slang IR must contain two canonical `IRGetOffsetPtr` instructions. Each result has
the same pointer type and access qualifier as its base; both use the exact signed-`i32` kernel
parameter as their element offset. The source offset feeds the established load, and the
destination offset feeds the established store. The direct emitter lowers each instruction through
one append-only provider operation that produces a non-`inbounds` LLVM `getelementptr i32` in
address space 1.

Real acceptance compiles the same source through direct NVVM and NVRTC, checks the raw PTX kernel
parameter widths `[64, 64, 32]` and global-memory behavior, assembles both outputs with `ptxas`, and
launches both routes. Runtime coverage copies from/to a positive array index and also passes
interior base pointers with index `-1`; the latter proves signed offset semantics without claiming
that the original allocation base may legally be indexed negatively.

## Progress

- [x] (2026-08-26) Completed and committed Slice 9 as `slice 9`.
- [x] (2026-08-26) Re-read `.agent/PLANS.md`, the Slice 9 hand-off, current provider/emitter
  boundaries, and the durable design/capability ledger.
- [x] (2026-08-26) Probed post-link/post-`simplifyIR` Slang IR for signed and unsigned pointer
  offsets and recorded the exact canonical shape and optimizer behavior.
- [x] (2026-08-26) Appended and implemented one coherent scalar-pointer-arithmetic provider
  capability with output sanitization, pre-mutation ownership/dominance/sized-pointee validation,
  and ordinary non-`inbounds` LLVM GEP.
- [x] (2026-08-26) Extended preflight and emission for exact signed-i32 device
  `IRGetOffsetPtr` instructions with result/base access-type equality and canonical value mapping.
- [x] (2026-08-27) Added ABI, provider validation/no-mutation, type-aware fake topology, exact
  Slice 9 gating, unsigned-literal boundary, real differential, `ptxas`, and positive/negative
  interior-base runtime tests; the initial complete prefix passed 68/68.
- [x] (2026-08-27) Ran the pinned formatter, rebuilt the Release provider and Debug host outside
  the sandbox, completed the helper/input-shape audit, updated the durable design and ledger, and
  passed the final 68/68 NVVM prefix plus every preservation lane.
- [x] (2026-08-27) Committed the 11 tracked files as `slice 10`, leaving all
  ExecPlans and `external/slang-binaries/` untracked, then began Slice 11.

## Surprises and Discoveries

- Observation: pointer addition remains canonical `IRGetOffsetPtr` through all observed optimizer
  and late-pass dumps.
  Evidence: the accepted source finishes as one
  `Func(Void, Ptr(Int,RW,UserPointer), Ptr(Int,Read,UserPointer), Int)` kernel with two offset
  instructions, one load, one store, and a void return. No explicit multiply or cast is introduced.
  Consequence: consume `IRGetOffsetPtr` directly; do not reconstruct byte arithmetic or syntax.

- Observation: access qualifiers remain part of the Slang result pointer type.
  Evidence: the destination offset result retains the read-write pointer type and the source result
  retains the read pointer type. Both otherwise match their bases exactly.
  Consequence: require `isTypeEqual(resultType, baseType)` in preflight. Existing load/store access
  checks remain the single source of truth for whether the derived pointer may be read or written.

- Observation: an unsigned source index remains `UInt` after optimization.
  Evidence: the unsigned probe did not acquire a signed cast or canonical signed-`i32` spelling.
  Consequence: reject it rather than silently reinterpret its bits. This slice accepts only the
  exact signed-`i32` value shape already supported by arithmetic, phis, calls, and returns.

- Observation: a raw `uniform uint` kernel index stops at the older entry-point ABI boundary before
  it can test the offset operand boundary.
  Evidence: the focused probe reported E52017 `'entry-point parameter'`. Replacing it with an
  explicit `uint(1)` offset while retaining only supported pointer kernel parameters reported the
  intended E52017 `'signed i32 value'`.
  Consequence: use the unsigned literal in retained-boundary coverage. It tests
  `IRGetOffsetPtr`'s value policy without conflating it with a new raw kernel parameter type.

- Observation: LLVM 14 typed pointers can still name unsized pointees that are invalid GEP element
  types.
  Evidence: `getPointerType` is broader than the new operation, while LLVM verification rejects GEP
  into an unsized element type.
  Consequence: the provider must require a non-opaque typed pointer with a sized pointee before
  mutation, even though Slang preflight narrows ordinary input to device `i32*`.

- Observation: the current public builder type constructors cannot safely produce an unsized
  pointee for a black-box provider test.
  Evidence: `getPointerType` already requires a loadable/storable pointee, and the private ABI has
  no struct-type constructor. Passing an invented LLVM handle would violate the opaque-handle
  contract and risk a crash rather than test a supported input.
  Consequence: retain and audit the explicit `isSized()` provider guard for ABI safety and future
  constructors, but do not fabricate an unsafe negative handle. Every currently constructible
  invalid shape remains covered through the public ABI.

## Decision Log

- Decision: Slice 10 is exactly signed-i32 element offsetting on the existing raw device `Ptr<int>`
  ABI.
  Rationale: it is the smallest independent memory-addressing capability and proves pointer-result
  SSA flow into load/store without conflating arrays, aggregates, globals, or another address space.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later slice deliberately adds `IRGetElementPtr`, other pointees, or another
  address space.

- Decision: append one V2 operation,
  `emitPointerOffset(module, basePointer, elementOffset, outPointer)`, as one coherent capability.
  Rationale: LLVM derives the result type from the typed base pointer; exposing a redundant result
  type would create a second source of truth. One operation is sufficient for both source and
  destination offsets.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an opaque-pointer provider ABI replaces the frozen LLVM 14 typed-pointer contract.

- Decision: emit ordinary, non-`inbounds` LLVM GEP.
  Rationale: Slang pointer addition establishes an element offset but this slice does not prove the
  stronger LLVM `inbounds` object/provenance contract. Negative-offset runtime coverage from an
  interior pointer is intentionally valid and should not inherit extra optimizer promises.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an upstream Slang invariant explicitly establishes LLVM-compatible inbounds
  provenance.

- Decision: the provider operation remains general over same-module scalar integer offsets and
  sized typed pointers, while Slang preflight owns this slice's device-i32/signed-i32 policy.
  Rationale: frontend subset policy belongs at the linked-IR acceptance boundary; the provider owns
  safe LLVM construction and should not bake in a temporary language/address-space restriction.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the private provider ABI itself needs a narrower portable type model.

## Outcomes and Retrospective

Slice 10 is complete in the `slice 10` commit. The accepted linked program is exactly one
`Func(Void, Ptr(Int,RW,UserPointer), Ptr(Int,Read,UserPointer), Int)` kernel. It contains two
canonical `IRGetOffsetPtr` instructions with exactly two operands each. Both consume the same
signed-i32 parameter; the source-derived pointer feeds the load, and the destination-derived
pointer feeds the store. The derived types equal their respective base types, including access,
and the shape survives optimization without a multiply, cast, or `IRGetElementPtr` rewrite.

The append-only V2 table is 248 bytes on this 64-bit build. Its new terminal minimum follows the
frozen 240-byte Slice 9 scalar-function minimum; earlier frozen minima remain 128, 168, 200, and
224 bytes. Negotiation accepts exact old and future-larger prefixes, rejects a byte count inside
the new field and a null complete field, sanitizes failed outputs, and records
`scalar-pointer-arithmetic=0|1` in the provider identity. An exact Slice 9 provider continues to
compile every old shape and rejects only the new shape before builder-module creation.

The LLVM 14 provider validates every input before mutation: live module, current unterminated
insertion block, output storage, same-module/function/dominating values, typed non-opaque pointer,
sized pointee, and scalar integer offset. The verified fixture contains exactly two ordinary
address-space-1 `getelementptr i32` instructions and no `inbounds`. Public constructors cannot
safely produce an unsized pointee, so the black-box matrix tests the closest public rejection
boundary (`getPointerType` rejects `void`) and the plan explicitly does not claim execution of the
internal `isSized()` guard.

The final formatted Release provider and Debug host builds passed. The authoritative NVVM prefix
passed 68/68. Preservation passed option parsing 1/1, routing/hash 2/2, unsupported IR 1/1,
default/explicit NVRTC sampler coverage 3/3, true NVRTC pass-through 2/2, and CUDA runtime dispatch
1/1. Direct NVVM and NVRTC reported matching `[64, 64, 32]` kernel parameter widths and global
load/store semantics; CUDA 12.9.86 `ptxas` accepted both routes. Both runtime routes copied the
intended positive-index element and the `-1` element from interior bases while preserving every
neighbor sentinel. `dumpbin` reports only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; the provider depends on no process-visible LLVM DLL.

The final principled-change inventory found no compensating representation repair:

- `_supportsScalarPointerArithmetic` survives as the single host-side classifier for the coherent
  appended ABI prefix. It consumes the provider's canonical size/function table and does not infer
  capability from generated IR.
- `_emitPointerOffset` survives at the provider construction boundary. Its input is the canonical
  typed LLVM pointer and scalar offset supplied through the private ABI; validating ownership,
  availability, and pointee legality there is provider safety, not a repair of Slang IR.
- The two `kIROp_GetOffsetPtr` cases survive in preflight and emission because the linked producer
  intentionally preserves exactly that canonical instruction for the motivating source. Preflight
  reuses `isTypeEqual`, `_validatePointerValue`, and `_validateI32Value`; emission reuses the
  canonical value map. Neither case reconstructs syntax, access, or byte arithmetic.
- Fake pointer-result identity/type helpers and the provider/runtime module builders survive only
  as test infrastructure needed to prove exact producer-consumer topology, rejection without
  mutation, PTX equivalence, and execution. The fake integer classifier was tightened to enumerate
  only known i32-producing value kinds, so future Boolean or pointer value kinds cannot be admitted
  accidentally.

The revert responsibility is direct: removing the ABI/provider operation fails the verified
builder and capability tests; removing the preflight/emission cases restores E52017 for the
ordinary source; removing canonical result mapping breaks the derived-pointer load/store topology.
Those failures demonstrate that each change sits at its producer/consumer boundary rather than
masking a malformed alternative representation.

## Context and Current Pipeline

`source/slang/slang-emit.cpp` links and optimizes the selected program, preserves raw CUDA
signatures for explicit direct NVVM, and calls `validateNVVMSupportedIR` before optional builder
discovery. `source/slang/slang-emit-nvvm.cpp` validates the finite function closure, selects the
minimum shape-dependent `NVVMIRCapability`, negotiates the builder, declares functions/parameters,
creates blocks and phis, emits bodies, attaches phi incoming edges, marks only the selected kernel,
and serializes once.

Slice 9's scalar values flow through `NVVMValueMap`. `_validatePointerValue` already checks the
canonical raw device-i32 pointer and its access requirement. `_validateI32Value` checks exact
signed-i32 values and dominance/availability. `_getLoweredNVVMValue` returns an already mapped
parameter/instruction or materializes a canonical integer literal. Slice 10 should reuse these
paths: validate the base with read access not required, validate the offset as i32, map the derived
pointer result, and let the existing load/store checks enforce access.

`source/compiler-core/slang-nvvm-ir-builder-api.h` freezes V1 and all prior V2 minima. Slice 9 ends
with `emitIntegerCall` and `emitIntegerReturn`. The host wrapper classifies coherent prefixes,
sanitizes output handles, and includes capability bits in the provider identity used by shader
cache keys. The LLVM 14 provider owns all LLVM objects in `ModuleState`; `setInsertBlock` selects
the unique current insertion block, and existing ownership/dominance helpers validate values before
mutation.

The current direct route reports E52017 `'getOffsetPtr'` before builder discovery for the motivating
source. That is the focused failing boundary this slice promotes to positive. `IRGetElementPtr`,
array/global/shared-memory representation, non-device address spaces, unsigned or wider indices,
and non-i32 pointees remain separate shapes.

## Scope and Non-Goals

In scope:

- canonical `kIROp_GetOffsetPtr` with exactly two operands;
- base/result types exactly equal and within the existing device `Ptr<int>` subset;
- exact signed-i32 element offsets, including negative values;
- derived pointer SSA values consumed by established load/store instructions;
- one append-only provider capability with old-prefix compatibility;
- fake, verified LLVM, differential PTX, `ptxas`, and CUDA runtime evidence.

Explicitly out of scope:

- `IRGetElementPtr`, arrays, vectors, structs, tuples, or field addressing;
- shared, local, constant, generic, or any other new address space;
- pointer helper parameters/results beyond Slice 9, pointer comparisons, pointer casts, pointer
  subtraction, byte addressing, or pointer-to-integer conversion;
- unsigned, 64-bit, or other index types and non-i32 pointees;
- `inbounds` promises, bounds checking, allocation provenance, or sanitizer behavior;
- shared-memory globals, aggregate kernel ABI, libdevice declarations, resources, or builtins.

## Architecture and Invariants

The linked Slang IR remains the semantic source of truth. Preflight accepts an offset only when the
instruction is exactly `IRGetOffsetPtr`, the result/base pointer types are identical supported
device-i32 pointer types, and the offset is an available signed-i32 value. It never converts a
checked semantic pointer back into an expression, walks arbitrary operands to infer access, or
repairs a malformed alternative representation.

Capability selection is monotonic. `ScalarPointerArithmetic` is a terminal enum level after
`ScalarFunctions`; a module containing `IRGetOffsetPtr` requires the complete new prefix. An exact
Slice 9 provider still compiles every Slice 9 shape and rejects only pointer-offset input as E52016
after discovery but before module creation. Every older minimum remains byte-for-byte frozen, and
partial or null-member new prefixes are rejected as incompatible.

The provider owns LLVM safety, not Slang policy. Before inserting anything it validates: a live
module; a valid current unterminated insertion block; an output pointer; a same-module typed,
non-opaque base pointer available at that insertion point; a sized pointee; and a same-module scalar
integer offset available at that insertion point. It then calls LLVM 14
`IRBuilder::CreateGEP(pointeeType, base, offset)` without `inbounds`. On failure it clears the output
and leaves the module unchanged.

The fake builder must model pointer-offset results as first-class provider values. It records the
caller block, base value reference, offset value reference, and result identity. Fake load/store
validation accepts either an original function parameter or a recorded pointer-offset result, so
the topology test can prove source offset -> load and destination offset -> store rather than merely
counting operations.

## Interfaces and Dependencies

Append to `SlangNVVMBuilderAPI_V2` after Slice 9:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitPointerOffset_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 basePointer,
    SlangNVVMValueHandle_1 elementOffset,
    SlangNVVMValueHandle_1* outPointer);
```

Publish one frozen minimum:

```c
SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE
```

Add host methods `supportsScalarPointerArithmetic()` and `emitPointerOffset(...)`; add stable
identity text `scalar-pointer-arithmetic=0|1`; and add `NVVMIRCapability::ScalarPointerArithmetic`.
No public Slang API changes. The implementation continues to depend on the optional statically
linked LLVM 14 provider, CUDA libNVVM/NVRTC, CUDA 12.9 `ptxas`, and the CUDA driver for runtime
evidence.

## Milestones

1. Freeze the provider suffix in `source/compiler-core/slang-nvvm-ir-builder-api.h`, update the C
   compile probes, host wrapper negotiation/identity/forwarder, provider getter, and exact older
   prefix tests. Verify partial, missing-member, larger-table, and failed-output behavior before
   touching Slang emission.

2. Implement provider `_emitPointerOffset` in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`. Add verified positive LLVM assembly and invalid
   no-mutation cases for null output, no/post-terminator insertion point, non-pointer base,
   non-integer offset, foreign/cross-function/non-dominating values, opaque/unsized pointee, and
   type/ownership mismatches. Exercise opaque/unsized pointees through the public ABI only if a
   valid constructor exists; otherwise record the `isSized()` guard as an audited future-proofing
   boundary rather than inventing an opaque handle.

3. Extend `source/slang/slang-emit-nvvm.h/.cpp` and the capability gate in
   `source/slang/slang-emit.cpp`. Accept only the exact final `IRGetOffsetPtr` shape, validate it in
   the existing per-function SSA order, emit it through the builder, and preserve the resulting
   pointer in the canonical value map. Do not add access reconstruction or a second pointer type.

4. Extend the fake builder and source fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove two offsets use `(kernel param 0,
   kernel param 2)` and `(kernel param 1, kernel param 2)`, the load consumes the source result, the
   store consumes the destination result, and the return/kernel annotation topology remains exact.
   Pin unsigned index and `IRGetElementPtr`-shaped work as unsupported where a stable source fixture
   exists.

5. Add real NVVM/NVRTC differential PTX, `ptxas`, and runtime array-copy evidence. Exercise a
   positive index on allocation bases and `-1` on interior bases, verify copied values and untouched
   neighbors for both routes, and skip only through the established environment gates.

6. Run the complete validation matrix, pinned formatter, binary inspection, and required
   principled-change audit. Update `docs/design/nvvm-backend.md` and
   `docs/design/nvvm-backend-capability-ledger.md` with demonstrated claims only. Keep this plan
   untracked, commit exactly the tracked Slice 10 files as `slice 10`, and immediately begin Slice
   11 unless a genuine blocker remains.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. All CMake builds and tests run outside the
sandbox as required by `AGENTS.md`.

Build:

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm
cmake.exe --build build --config Debug --target slang-test
```

Focused provider-independent and real-provider tests use exact names added during implementation.
The authoritative prefix is:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Established preservation regressions:

```text
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/coverageCudaRuntimeDispatch
```

Acceptance requires:

- the final linked IR has exactly two same-type `IRGetOffsetPtr` values with the shared signed-i32
  index, followed by source-result load and destination-result store;
- fake topology proves exact function/block/base/offset/result consumers, not only operation counts;
- provider LLVM assembly contains two ordinary address-space-1 `getelementptr i32` instructions and
  no `inbounds`, and verification diagnostics are empty;
- invalid provider inputs and partial prefixes fail without output handles or partial LLVM mutation;
- exact Slice 9 providers retain all older shapes and reject the new shape only at E52016 after
  discovery; short-buffer provider discovery remains compatible;
- direct NVVM and NVRTC expose `[64, 64, 32]`, agree on global-memory semantics, and both assemble;
- both routes copy the intended element for positive and negative interior-base offsets and leave
  neighboring sentinels unchanged;
- unsigned indices, `IRGetElementPtr`, other address spaces/pointees, aggregates, and shared memory
  remain deterministic unsupported boundaries;
- the full NVVM prefix and preservation set pass after formatting;
- provider exports remain only V1/V2 getters, no process-visible LLVM DLL dependency appears,
  pinned clang-format makes no changes, and `git diff --check` passes.

## Failure and Recovery

The IR probe, builds, tests, formatter diff check, binary inspection, and status commands are safe
to repeat. Provider and host builds are incremental. The optional builder path is set only in the
current PowerShell process. Failed output-handle and no-mutation tests distinguish host/provider ABI
errors from LLVM verification or libNVVM failures.

Do not delete/reset the user's worktree or stage `external/slang-binaries/` or any ExecPlan. If
ordinary source does not retain the probed `IRGetOffsetPtr`, re-probe the producer and update this
plan; do not accept byte arithmetic or `IRGetElementPtr` as a fallback. If LLVM rejects a provider
module, fix validation/construction before serialization rather than masking verifier output. The
experimental route remains removable without affecting default NVRTC dispatch.

## Artifacts and Hand-Off

Retain in this plan: exact final linked IR, API prefix sizes, verified LLVM assembly excerpts,
negative/no-mutation matrix, PTX parameter widths/global operations, `ptxas` versions/results,
runtime base/index/value/sentinel observations, final test counts, exports/dependencies, and the
required inventory/input-shape audit for every new helper or special case.

Temporary probe sources may live under `issue-nvvm-backend/` while active and must be removed before
the Slice 10 commit. Distill durable architecture into `docs/design/nvvm-backend.md`, durable test
status into `docs/design/nvvm-backend-capability-ledger.md`, and the implementation narrative into
the eventual five-part PR description. Keep this plan and prior slice plans untracked.
