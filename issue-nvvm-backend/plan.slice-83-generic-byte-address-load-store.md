# Add generic byte-offset addressing and core byte-buffer operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM supports the core 32-bit `ByteAddressBuffer` and
`RWByteAddressBuffer` load family plus the matching `RWByteAddressBuffer` store used by the existing
`tests/compute/byte-address-buffer.slang`. Scalar UInt and UInt2 through UInt4 accesses keep byte
offset and promised-alignment semantics, read-only loads remain invariant, and all physical memory
operations compose through one generic byte-offset pointer builder operation plus the established
generic load/store surface.

The existing shader must pass its established lanes and new direct-libNVVM CUDA runtime lanes for
both read-only and read-write input resources. Direct PTX must expose accepted global scalar/vector
loads and stores and pass CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Completed and committed Slice 82 as `3f233f97f` with 351/351 NVVM tests.
- [x] (2026-08-29) Reproduced the existing shader's E52017 `byteAddressBufferLoad` boundary.
- [x] (2026-08-29) Captured final linked IR: repeated UInt/UInt2/UInt3/UInt4 canonical byte loads,
  one UInt byte store, established helper calls, vector operations, conventional resource fields,
  and CUDA execution-vector extraction.
- [x] (2026-08-29) Added exact ABI revision 8 generic byte-offset pointer construction and
  provider validation.
- [x] (2026-08-29) Added canonical byte-load/store shape, SSA, access, alignment, and generic
  emission support.
- [x] (2026-08-29) Registered provider/fake/negative and existing-file runtime/PTX/`ptxas`
  evidence.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable
  documents, self-reviewed, and prepared the completed slice for commit.

## Surprises and Discoveries

- The default existing shader needs no new arithmetic or vector family. Its final linked graph has
  one reachable scalar helper, established UInt3 execution-vector extraction, wrapping UInt
  arithmetic, UInt2/UInt3/UInt4 loads and swizzles, and one UInt store. The first direct stop is the
  first canonical byte load.
- Canonical byte loads have two or three operands after target legalization. The optional third
  operand is a literal alignment promise; zero means the ordinary four-byte contract. Canonical
  stores consistently carry buffer, byte offset, alignment literal, and value.
- Reusing element-based pointer offset would require recognizing and undoing source multiplication
  by four. That would be a consumer-side expression special case and would fail for arbitrary byte
  offsets. The missing LLVM capability is byte-granular pointer addressing with a typed result.
- The aligned source overloads in the existing shader are canonicalized upstream to two-operand
  byte loads. The direct emitter therefore receives the default four-byte contract for those
  operations. Retained three-operand canonical loads still forward an explicit positive
  power-of-two alignment, and the provider test covers a 16-byte vector load.
- The existing shader's `void test(uint)` helper exposed a pre-existing producer-consumer mismatch.
  Signature preflight already admitted void helpers, but the shared return validator and emitter
  treated every helper return as a scalar. The canonical void literal now validates at that shared
  boundary and void helpers emit `ret void`.

## Decision Log

- Decision: bump the exact forward-only builder ABI to revision 8 and add
  `emitByteOffsetPointer(module, basePointer, byteOffset, resultPointeeType, outPointer)` to the
  construction interface.
  Rationale: the operation is an LLVM memory-address primitive rather than a Slang resource
  operation. It preserves the base pointer's address space, performs byte-granular addressing, and
  returns a pointer to an explicit load/store type. Byte-buffer emission can then reuse generic
  struct extraction and load/store operations.
  Date/author: 2026-08-29, Codex.
- Decision: admit exact unsigned-i32 scalar and two- through four-lane vector byte accesses in this
  slice.
  Rationale: this is the coherent core HLSL `Load`, `Load2`, `Load3`, `Load4`, and scalar `Store`
  family exercised by the existing test. Generic signed/float/64-bit/aggregate operations undergo
  additional bit reinterpretation or scalarization and remain separate measured boundaries.
  Date/author: 2026-08-29, Codex.
- Decision: normalize a missing or zero alignment promise to four bytes and otherwise forward the
  exact positive power-of-two literal.
  Rationale: core byte-buffer operations guarantee four-byte addressing, while aligned overloads
  explicitly promise stronger alignment. The provider must receive the contract, not infer
  alignment from constant offsets or the LLVM vector's preferred alignment.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 83 completes the core unsigned 32-bit byte-address family without adding a resource-specific
LLVM callback. ABI revision 8 provides one generic byte-offset pointer primitive; byte-buffer
emission composes it with established field extraction, typed loads/stores, alignment, and
invariant-load policy. The fake-provider tests prove three exact pointer constructions across
scalar and vector types, read-only versus read-write load policy, and store topology. Adjacent
float access still fails before provider discovery.

The three exact lanes added to `tests/compute/byte-address-buffer.slang` pass 3/3. Both read-only
and read-write direct PTX modules compile through the real libNVVM and CUDA 12.9.86
`ptxas -arch=sm_70`. The standalone provider and Release `slang-unit-test`, `slang-test`, and
`slangc` builds pass. The complete `slang-unit-test-tool/nvvm` prefix passes 353/353. Pinned
clang-format 17 and `git diff --check` pass. The unrelated untracked
`external/slang-binaries/` directory remains unstaged.

Self-review inventoried the new byte-access descriptor, unsigned-i32 validator, core value-type
classifier, and builder callback. Each survives: the descriptor consumes the canonical operation
shape rather than rediscovering source syntax, the validator is shared with wave-mask validation,
the classifier is the single exact type boundary, and the callback is a generic LLVM pointer
primitive. The void-helper change fixes the shared return boundary that produced the mismatch; it
does not patch the byte-access consumer or reconstruct a second representation.

## Context and Current Pipeline

Consider the existing source:

```slang
uint tmp = inputBuffer.Load(uint(val * 4));
uint2 pair = inputBuffer.Load2(uint(tmp * 4));
uint3 triple = inputBuffer.Load3(0);
uint4 alignedQuad = inputBuffer.Load4Aligned(16, 16);
outputBuffer.Store(val * 4, tmp);
```

Conventional-global collection produces a 32-byte struct containing one read-only or read-write
input byte-address view and one read-write output view. Each view already lowers through Slice 82's
`{ i32 addrspace(1)*, i64 }` representation. The final body loads a view, performs
`byteAddressBufferLoad(buffer, byteOffset[, alignment])`, consumes scalar/vector results through
established operations, and performs `byteAddressBufferStore(buffer, byteOffset, alignment,
value)`.

Direct preflight currently rejects the first load before builder discovery. Field zero already
provides the physical global `i32*`; the emitter needs a byte-addressed pointer to the exact scalar
or vector result, after which existing aligned load/store operations own memory emission.

## Scope and Non-Goals

In scope are exact `ByteAddressBuffer` and `RWByteAddressBuffer` source views; exact UInt,
UInt2, UInt3, and UInt4 load results; exact UInt, UInt2, UInt3, and UInt4 values for read-write
stores when canonical IR produces them; unsigned-i32 byte offsets; zero/default or positive
power-of-two literal alignment; canonical direct operations; raw and conventional resource
producers; and read-only/invariant versus read-write load policy.

Out of scope are rasterizer-ordered buffers; signed, narrow, wide, float, Boolean, pointer,
aggregate, matrix, or float-vector values; atomics; status-returning loads; runtime alignment;
misalignment repair; bounds; dimensions; resource arrays; pointer escape from the produced typed
address; helper ABI changes; integer shifts needed by legalized 64-bit access; and other address
spaces beyond what provider validation generically permits.

## Architecture and Invariants

One byte-access descriptor resolves only canonical load/store operations. It requires Slice 82's
exact byte-address resource descriptor, exact unsigned-i32 byte offset, exact admitted value type,
and a literal alignment. Loads accept read-only or read-write views; stores require read-write.
Zero alignment normalizes to four. No matcher searches arithmetic expressions or rewrites an
element index.

The builder operation accepts one usable typed pointer, one usable integer byte offset, and one
module-context-owned loadable result pointee type at a valid insertion point. The provider bitcasts
the base to same-address-space `i8*`, emits a non-`inbounds` byte GEP, and bitcasts to a pointer to
the requested type. It rejects invalid/foreign/unavailable handles before mutation.

Emitter load/store paths extract raw-buffer field zero, request the generic byte-offset pointer,
then invoke established `emitLoad`/`emitStore` with the resolved alignment. Only the resource
producer determines invariant-load policy.

## Interfaces and Dependencies

Update `source/compiler-core/slang-nvvm-ir-builder-api.h`, `slang-nvvm-ir-builder.{h,cpp}`, the real
provider, and fake provider for exact ABI revision 8 and one required construction callback. Add
normal and LLVM-7-compatible provider assembly checks plus invalid-operation coverage.

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` only for one exact core byte-access
value classifier. Update `source/slang/slang-emit-nvvm.cpp` for descriptor validation, capability
requirements, SSA availability, and generic emission. Do not add a resource-named method to the
facade or provider.

Register direct runtime and PTX evidence in `tests/compute/byte-address-buffer.slang`. Extend the
fake emitter tests with exact buffer/offset/type/alignment/load/store topology and retain adjacent
unsupported signed/float/64-bit or rasterizer-ordered controls before provider mutation.

## Milestones

1. Add and negotiate exact ABI revision 8 byte-offset pointer construction in the facade, fake,
   and real provider.
2. Verify normal/compatible LLVM construction and reject invalid provider inputs before mutation.
3. Add exact canonical core byte-load/store validation and compose emission from field extraction,
   byte addressing, and generic memory operations.
4. Prove fake topology, alignment/access behavior, and adjacent deterministic boundaries.
5. Register the existing shader's direct CUDA runtime and PTX lanes and run CUDA 12.9 `ptxas`.
6. Run focused and complete NVVM tests, update design/ledger records, self-review, and commit this
   plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- exact ABI revision 8 rejects revision mismatch and any missing construction callback;
- provider normal and LLVM-7-compatible text contains same-address-space byte-pointer casts, a
  non-`inbounds` i8 GEP, and the requested typed result pointer;
- invalid module/type/value/insertion/dominance/address-space shapes fail with sanitized outputs
  before provider mutation;
- fake emission observes generic field extraction, byte-offset pointers, exact four/stronger-byte
  alignments, invariant read-only loads, ordinary read-write loads, and stores;
- unsupported adjacent resource/value/alignment shapes retain deterministic E52017 before builder
  discovery;
- both existing shader variants pass direct GPU comparison with the established results;
- direct PTX passes FileCheck and CUDA 12.9 `ptxas -arch=sm_70`;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects the pointer casts, inspect both normal LLVM 14 and compatible LLVM 7 text. The
provider operation must preserve the base address space and use typed pointers throughout; do not
fall back to integer pointer arithmetic or resource-specific IR text. If vector runtime results
differ, inspect the forwarded alignment first and compare wide versus scalarized final IR; do not
infer alignment from a constant location.

All ABI, facade, provider, emitter, fake, test, and document changes are one forward-only slice and
can be reverted together. The established element-offset operation and Slice 82 escaped-pointer
path remain independent.

## Artifacts and Hand-Off

Retain provider normal/compatible assembly, direct PTX for both resource variants, runtime output,
and `ptxas` artifacts under ignored `build/` paths. Distill the generic byte-offset pointer
contract, canonical byte-access relation, alignment policy, existing-file results, remaining
generic/64-bit/atomic boundaries, and exact validation totals into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md` before committing
Slice 83.
