# Carry fixed numeric arrays through local helper references

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, fixed nonempty arrays of supported numeric scalar/vector elements can live in
generic local storage, cross direct helper boundaries through exact `out` or `inout` references,
and produce typed element pointers for constant or dynamic i32 indices. The existing
`array-param.slang` fixture runs through direct libNVVM with its shader body and expected output
unchanged.

## Progress

- [x] (2026-08-29) Measured `array-param.slang` and neighboring matrix fixtures after Slice 108.
- [x] (2026-08-29) Defined one local fixed-numeric-array pointer classifier shared by preflight, type lowering,
  call compatibility, and element-address validation.
- [x] (2026-08-29) Admitted local array allocation and exact local-`Ptr` to array `OutParam`/`BorrowInOutParam`
  helper calls using the existing generic pointer and array element operations.
- [x] (2026-08-29) Added focused fake-provider positive coverage and retained nested/unsupported array negatives.
- [x] (2026-08-29) Promoted `array-param.slang` to direct runtime and PTX coverage.
- [x] (2026-08-29) Built, ran focused and complete NVVM tests, assembled PTX, self-reviewed, updated durable status
  and this plan, and prepared the complete slice commit.

## Surprises and Discoveries

- `asNVVMSupportedNumericArrayType`, provider array types, generic local allocation, aggregate
  load/store, and `emitArrayElementPointer` already exist. The missing boundary is a compiler-side
  pointer classifier and relation check; no new builder callback is required.
- `array-param.slang` final IR is compact: one `BorrowInOutParam(float3[4])`, one local array, four
  constant element stores in the helper, and one element load in the kernel.
- `column-major.slang` also contains `OutParam(float4[4])`, but additionally indexes through a
  pointer-to-vector to obtain a scalar address and reads array-valued constant buffers. Those are
  distinct aggregate-address and resource-layout boundaries, so admitting its signature alone
  would merely move the failure.
- The canonical local-array base and result deliberately use different pointer spellings. A `Var`
  or helper reference is the compact one-operand generic pointer classified for the ABI, while
  `IRGetElementPtr` produces a four-operand pointer carrying CUDA `ScalarLayout`. Keeping those
  roles separate avoided broadening helper signatures to every decorated numeric pointer.

## Decision Log

- Decision: add a local numeric-array pointer classifier parallel to the existing local numeric and
  local copyable-struct pointer classifiers.
  Rationale: canonical `Ptr`, `OutParam`, and `BorrowInOutParam` types carry the same selected array
  pointee and generic address space while preserving their source ownership spelling. One
  classifier prevents signature, lowering, and address validation from drifting.
  Date/author: 2026-08-29, Codex.
- Decision: reuse the provider's typed generic pointer, local allocation, and array GEP operations.
  Rationale: the provider API already represents the full selected operation. A local-specific or
  helper-specific callback would duplicate address-space and pointee facts already in the handles.
  Date/author: 2026-08-29, Codex.
- Decision: keep nested vector-component addressing and constant-buffer arrays out of this slice.
  Rationale: they require different producer relations and cannot be proved by the array helper
  fixture.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Fixed numeric arrays are accepted as first-class `NVVMTypeUse::Value` aggregates and as basic-block
parameters. The provider constructs LLVM array types, can allocate any admitted local value type,
and emits a typed non-`inbounds` GEP from an array pointer and scalar integer index. Device i32
array pointers and shared i32 arrays already use that operation, but compiler preflight currently
limits local `Var` to numeric values or flat copyable structs and helper references to those same
pointee families.

`array-param.slang` lowers `float3 b[4]` to `Ptr(Array(float3, 4))`, passes it to
`writeArray(BorrowInOutParam(Array(float3, 4)))`, writes each selected element, then loads element
zero for its existing structured-buffer output.

## Scope and Non-Goals

In scope:

- nonempty fixed arrays selected by `asNVVMSupportedNumericArrayType`;
- generic local `Ptr`, `OutParam`, and `BorrowInOutParam` array pointers;
- exact local-`Ptr` call compatibility for array outputs and mutable borrows;
- local allocation alignment derived from the selected numeric element;
- array element addresses with selected i32 indices and exact element pointer types;
- fake-provider, real-provider, runtime, PTX, and negative coverage.

Out of scope:

- nested arrays, arrays of structs/resources/matrices, empty or unsized arrays;
- device entry array families beyond the established fixed i32 subset;
- vector-component, matrix-row/column, or nested aggregate element addresses;
- first-class array helper parameters/results or array phis beyond already-admitted internal value
  transport;
- parameter-block or constant-buffer array storage;
- bounds checks or LLVM `inbounds` provenance.

## Architecture and Invariants

- `asNVVMSupportedNumericArrayType` remains the sole selected array-value definition.
- Local array pointer forms must have exactly that pointee, generic address space, one canonical
  operand, and only `Ptr`, `OutParam`, or `BorrowInOutParam` ownership.
- A local pointer satisfies an output/mutable array helper parameter only when their canonical
  array pointees are exactly equal.
- Element addressing validates base, result, address space, and exact element identity before
  provider mutation; the index remains a selected i32 value.
- The local allocation alignment is the array element's existing numeric alignment.
- Unsupported neighboring shapes stop during preflight.

## Interfaces and Dependencies

No production builder ABI revision is planned. Add the compiler classifier and use the existing
`getArrayType`, `getPointerType`, `emitLocalStorage`, `emitArrayElementPointer`, `emitLoad`, and
`emitStore` operations. Extend the fake provider only where it has a narrower test-only assumption
than those generic operations. Update `array-param.slang`, durable design status, and this plan.

## Validation and Acceptance

Acceptance requires focused fake-provider coverage for local array allocation, array helper
reference transport, and element stores/loads; retained nested-array and device-array negatives;
the exact existing `array-param.slang` ordinary and direct runtime lanes; direct PTX; CUDA 12.9
`ptxas -arch=sm_70` acceptance; the complete `slang-unit-test-tool/nvvm` prefix; pinned
clang-format; and `git diff --check`.

Record exact counts, output, PTX/cubin sizes, the next boundary, and self-review.

## Self-Review and Input-Shape Audit

Inventory the new classifier and every widened branch. Trace the concrete `Ptr(float3[4])` producer
from local `Var` through the helper call and each `IRGetElementPtr`. Confirm the pointer ownership
spellings are canonical and intentional, the provider sees one typed generic pointer, and no code
walks operands to rediscover the array. Remove each widening when practical to identify its focused
failure. Do not retain a matrix, resource, or nested-aggregate special case.

## Failure and Recovery

If LLVM verification, libNVVM, runtime, or `ptxas` rejects the selected local array, preserve output
under ignored `build/` and stop at the real boundary. Do not weaken array classification, invent a
fixture-specific callback, reset unrelated work, or stage `external/slang-binaries/`.

## Outcomes and Retrospective

Slice 109 establishes fixed selected numeric arrays as a complete local reference family without a
provider ABI change. `asNVVMSupportedLocalNumericArrayPointerType` is the single source of truth for
compact `Ptr`, `OutParam`, and `BorrowInOutParam` ownership forms. `_getNVVMLocalArrayElementPointer`
owns the canonical decorated GEP relation and requires the exact base array, direct numeric element,
generic read-write address, CUDA scalar layout, and i32 index. The local allocation path reuses the
existing array type and element alignment; the emitter reuses `emitArrayElementPointer`.

The focused fake-provider test passes with `out float3[4]` and `inout float3[4]` helpers, one
16-byte-aligned local array, two calls, six typed array element pointers, one load, and six stores.
The retained unsupported-IR test rejects a nested local array and neighboring device/nested array
shapes before provider mutation. The exact new `array-param.slang` direct runtime lane passes with
output `1, 1, 1, 1`, and its direct PTX FileCheck lane passes. Standalone optimized PTX is 645 bytes;
CUDA 12.9.86 `ptxas -arch=sm_70` accepts it and emits a 2,792-byte cubin. The Release build is clean,
the complete NVVM unit prefix passes 378/378, pinned formatting and `git diff --check` pass, and no
provider ABI surface changed.

The self-review inventory retains one value-family classifier, one producer/result resolver, and
the corresponding role widenings. Removing the classifier reproduces helper/local allocation
failure; removing the resolver reproduces the measured `device i32 array element pointer` failure.
Both input shapes are canonical products of `IRVar`, parameter lowering, and `IRGetElementPtr`; no
syntax is rebuilt and no operand graph is searched. Nested arrays, array-valued helpers,
pointer-to-vector scalar GEPs, and constant-buffer arrays remain outside this slice. The next
measured boundary should start from the matrix fixtures' nested vector-component address and
array-valued constant-buffer shapes rather than adding another array ABI exception.
