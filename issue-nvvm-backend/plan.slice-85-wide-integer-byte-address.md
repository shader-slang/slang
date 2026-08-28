# Admit 64-bit integer byte-address access

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, canonical byte-address loads and read-write stores admit exact signed and
unsigned 64-bit integer scalars alongside the selected 32-bit numeric family. They compose from the
existing generic byte-offset pointer, typed load/store, selected-integer arithmetic, and conversion
operations; builder ABI revision 8 does not change.

The existing `tests/compute/byte-address-buffer-64bit.slang` must compile through direct libNVVM,
execute through the CUDA comparison harness, expose the complete UInt64 store lowering in direct
PTX, and pass CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Completed and committed Slice 84 as `6fee23603` with 354/354 NVVM tests.
- [x] (2026-08-29) Reproduced E52017 `core byte-address buffer access` on the existing 64-bit
  shader before provider discovery.
- [x] (2026-08-29) Captured the final linked shape: UInt load, UInt-to-UInt64 conversion, UInt64
  add, and one canonical UInt64 byte-address store.
- [x] (2026-08-29) Defined the exact byte-address scalar boundary from the established numeric type
  families.
- [x] (2026-08-29) Added focused signed/unsigned 64-bit load/store topology and an adjacent
  aggregate rejection.
- [x] (2026-08-29) Registered the existing shader for direct runtime/PTX evidence and ran real
  libNVVM/`ptxas`.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable documents,
  self-reviewed,
  and prepared the completed slice for commit.

## Surprises and Discoveries

- Slice 68 already admits Int64/UInt64 as ordinary values, constants, conversions, wrapping
  arithmetic, helpers, and naturally aligned device memory. Slice 83's generic byte-offset pointer
  is pointee-type agnostic. The only observed rejection is Slice 84's byte-access classifier.
- The motivating file does not load a 64-bit value. It loads UInt, widens it, adds one as UInt64,
  and stores UInt64. Focused coverage must independently exercise both wide loads and stores and
  both semantic signednesses.
- NVVM-target byte-address legalization preserves basic 64-bit integer accesses. Metal's separate
  `lowerBasicTypeOps` policy is the path that splits them into 32-bit operations; direct NVVM does
  not need to reconstruct or undo that target-specific shape.
- libNVVM retains the UInt64 add but lowers the four-byte-aligned i64 store to two ordered
  `st.global.u32` instructions with a `shr.u64` between them. PTX evidence must describe this actual
  lowering rather than require a `st.global.u64` instruction.

## Decision Log

- Decision: admit exact Int64 and UInt64 scalar byte-address values by composing the established
  selected-integer classifier with Slice 84's selected 32-bit numeric boundary.
  Rationale: these are already canonical ordinary values with real-provider type and operation
  support. The byte-access descriptor should state the semantic policy, while the generic pointer
  and memory operations continue to own physical emission.
  Date/author: 2026-08-29, Codex.
- Decision: do not admit narrow integers, 64-bit vectors, Float64, Boolean, or aggregates as a side
  effect.
  Rationale: the observed suite boundary is 64-bit integer scalar storage. Narrow byte-address
  layout, wider vector value support, floating-point policy, and recursive aggregate values need
  independent evidence rather than inference from LLVM permissiveness.
  Date/author: 2026-08-29, Codex.
- Decision: keep the default no-promise byte-address alignment at four bytes and preserve explicit
  literal alignment unchanged.
  Rationale: the source-level plain `Load`/`Store` contract supplies the existing four-byte minimum;
  an i64 LLVM load/store with alignment four is valid. Claiming natural alignment eight for a
  runtime byte offset would strengthen a promise the source did not make.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 85 admits exact Int64 and UInt64 byte loads/stores by widening one semantic classifier; ABI
revision 8 and the generic provider remain unchanged. The focused wide fixture proves read-only
invariant versus read-write ordinary loads, explicit-eight versus default-four-byte alignment, two
wide byte-buffer stores, and a retaining device-pointer store. The aggregate control still stops
before provider discovery. Four focused new/adjacent unit tests pass 4/4.

The existing 64-bit shader passes its direct runtime and PTX lanes 2/2. Real direct emission
preserves UInt64 arithmetic and lowers its four-byte-aligned wide store to two ordered UInt stores;
CUDA 12.9.86 `ptxas -arch=sm_70` accepts the module. The standalone provider and Release
`slang-unit-test`, `slang-test`, and `slangc` targets build successfully. The complete NVVM prefix
passes 355/355; pinned clang-format 17 and `git diff --check` pass.

The final self-review inventory contains one production generalization and two evidence changes.
`isNVVMSupportedByteAddressValueType` survives as the single semantic composition of already-
canonical type classifiers; it neither reconstructs syntax nor walks producers. The descriptor
continues to validate the canonical byte access before provider discovery and uses the existing
generic memory path. The focused fixture crosses valid signed/unsigned and alignment contracts,
while the aggregate fixture preserves a measured unsupported canonical shape. No fallback, cast,
source-name match, malformed-IR repair, or consumer-side representation workaround was added.

## Context and Current Pipeline

Consider the existing shader:

```slang
uint tmp = inputBuffer.Load(uint(val * 4));
uint64_t tmp64 = uint64_t(tmp) + 1;
outputBuffer.Store(uint(val * 8), tmp64);
```

Final linking preserves this as a canonical UInt byte load, explicit UInt-to-UInt64 `intCast`,
UInt64 `add`, and UInt64 byte store. Slice 68 already lowers the conversion and add using V4 typed
operation descriptors. Slice 83 extracts the raw byte-buffer data pointer, applies a UInt byte
offset with `emitByteOffsetPointer`, and emits a typed load/store. Slice 84 restricts the descriptor
to exact 32-bit numeric scalar/vector types, so preflight rejects the final UInt64 store before
loading the builder.

The focused source will cross signed and unsigned wide load/store uses so optimization cannot erase
the producer or consumer boundary. An aggregate byte access remains the control proving unsupported
recursive values still stop before provider mutation.

## Scope and Non-Goals

In scope are exact Int64 and UInt64 scalar loads from read-only or read-write byte buffers, matching
stores to read-write byte buffers, the established UInt byte offset, zero/omitted versus explicit
literal alignment, invariant-load policy, focused fake topology, the named existing shader, direct
PTX, runtime comparison, and `ptxas` validation.

Out of scope are Int8/UInt8/Int16/UInt16 byte access; 64-bit vectors; Float64, half, Boolean,
pointer, descriptor-handle, array, struct, matrix, interface, or resource values; rasterizer-ordered
buffers; status loads and atomics; runtime alignment; resource arrays; and bounds repair.

## Architecture and Invariants

One byte-address value classifier composes existing canonical type classifiers. It admits the
selected 32-bit numeric family plus exact 64-bit signed/unsigned integer scalars. It does not match
source names, inspect uses, or reinterpret another type.

The generic byte-offset pointer receives the exact lowered i64 pointee. Signedness remains semantic
above LLVM's signless i64, while the V4 operation descriptors preserve signed/unsigned conversion
and arithmetic behavior. Access kind solely controls invariant-load flags. A zero or omitted
alignment keeps the four-byte minimum; a nonzero power-of-two literal is forwarded exactly.

## Interfaces and Dependencies

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` to replace the 32-bit-only byte value
predicate with the explicit selected byte-address family. Update the existing descriptor in
`source/slang/slang-emit-nvvm.cpp` to use it. No provider, facade, semantic catalog, callback,
feature flag, ABI revision, or LLVM-compatible text rewrite is expected.

Replace Slice 84's temporary UInt64 negative with a positive focused source/test and retain an
aggregate negative under `tools/slang-unit-test/`. Add direct CUDA comparison and PTX lanes to the
existing file-backed shader. Keep generated IR/PTX/runtime/cubin artifacts under ignored `build/`.

## Milestones

1. Express the selected byte-address value family and update the canonical descriptor.
2. Add focused signed/unsigned i64 load/store topology and an aggregate pre-provider rejection.
3. Register `byte-address-buffer-64bit.slang` for direct CUDA comparison and direct PTX checks.
4. Compile through real libNVVM, inspect the 64-bit memory/conversion/arithmetic output, and run
   CUDA 12.9 `ptxas -arch=sm_70`.
5. Run focused regressions and the complete NVVM prefix, update durable design/ledger records,
   self-review, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- exact Int64/UInt64 byte loads and read-write stores reach generic byte-offset pointer and typed
  load/store operations with the exact alignment/load flags;
- read-only wide loads carry the invariant flag while read-write loads remain ordinary;
- an aggregate byte access remains deterministic E52017 before builder discovery or mutation;
- the existing shader's direct CUDA lane matches its established CPU result;
- direct PTX contains the expected UInt load, UInt64 add, and two-word lowering of the UInt64 store,
  passes FileCheck, and CUDA 12.9 `ptxas -arch=sm_70` accepts it;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects an i64 byte pointer or alignment four, compare normal LLVM 14 text, compatible
NVVM text, and NVRTC PTX before changing policy. Do not split the value in the direct emitter: an
i64 access is the canonical NVVM-target shape and target-specific splitting belongs in byte-address
legalization when requested by target options.

If the focused fake cannot distinguish semantic signedness because LLVM integer handles are
signless, assert the exact V4 conversion/operation descriptors and use real assembly/PTX for width;
do not add signed physical LLVM types. All host/fake/test/document changes are one forward-only
slice and can be reverted together; builder ABI revision 8 remains unchanged.

## Artifacts and Hand-Off

Retain final linked IR, direct PTX, CUDA runtime output, and `ptxas` artifacts under ignored
`build/nvvm-slice85/`. Distill the 64-bit scalar byte-access contract, existing-file results,
remaining narrow/vector/aggregate boundaries, and exact validation totals into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md` before committing
Slice 85.
