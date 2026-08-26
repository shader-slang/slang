# Address fixed device arrays through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 11 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts dynamic element addressing through device pointers
to nonempty fixed arrays of signed `i32`. The shortest observable source is:

```slang
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWIntArray4 destination,
    uniform RIntArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
```

The final linked Slang IR must contain two canonical `IRGetElementPtr` instructions. Each consumes
its device pointer-to-`int[4]` base and the exact shared signed-`i32` kernel parameter. Each result
is a device `Ptr<int>` with the same access qualifier as its base. The source result feeds one
load, and that load feeds the store through the destination result.

The provider represents the parameter pointee as LLVM `[4 x i32]` and lowers each element address
to an ordinary non-`inbounds` LLVM GEP with indices `{i32 0, index}` in address space 1. Real
acceptance compiles the source through direct NVVM and NVRTC, checks raw PTX parameter widths
`[64, 64, 32]` and global-memory behavior, assembles both routes with `ptxas`, and launches indices
0 and 3 while proving every neighboring destination sentinel remains unchanged.

## Progress

- [x] (2026-08-27) Completed Slice 10 as `slice 10` with its final 68/68 NVVM
  prefix and preservation matrix green.
- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 10 hand-off, current provider/emitter
  boundaries, and the durable design/capability ledger.
- [x] (2026-08-27) Probed device array pointers, local arrays, struct pointers, and constant arrays;
  selected the exact canonical fixed-device-array boundary and proved the fixture compiles through
  the established NVRTC route.
- [x] (2026-08-27) Froze and implemented the append-only scalar-array-addressing provider
  capability, including coherent-prefix negotiation, identity, sanitized wrappers, provider
  validation, and strict-C layout probes; the Release provider build is green.
- [x] (2026-08-27) Extended direct-NVVM preflight, parameter type caching/lowering, body
  emission, and pre-module capability gating for the exact fixed-i32-array shape; the Debug
  `slangc` integration build is green.
- [x] (2026-08-27) Added eight ABI, provider, fake-topology, capability-gate, differential PTX,
  `ptxas`, runtime, and negative tests, taking the focused prefix from 68 to 76 tests.
- [x] (2026-08-27) Applied pinned clang-format 17, rebuilt Release provider and Debug host outside
  the sandbox, passed 76/76 plus the preservation matrix, inspected the binary, completed two
  independent diff audits, updated both durable design documents, and committed the tracked slice
  as `slice 11`.

## Surprises and Discoveries

- Observation: the motivating device-array source remains canonical after `simplifyIR`.
  Evidence: the final entry signature is
  `Func(Void, Ptr(Array(Int,4),RW,UserPointer), Ptr(Array(Int,4),Read,UserPointer), Int)`. Its body
  has exactly two two-operand `getElementPtr` values, one load, one store, and a void return. The
  current direct route stops first at E52017 `'entry-point parameter'` because Slice 10 accepts only
  device pointers directly to `i32`.
  Consequence: consume the existing `IRArrayType` and `IRGetElementPtr` representation directly;
  do not rewrite it into `IRGetOffsetPtr`, byte arithmetic, or syntax-derived layout.

- Observation: the destination and source element pointers preserve their bases' access while
  intentionally changing pointee and data-layout spelling.
  Evidence: the destination result is read-write device `Ptr(Int, ScalarLayout)`, the source result
  is read-only device `Ptr(Int, ScalarLayout)`, while both bases point to `Array(Int,4)` with
  `DefaultLayout`.
  Consequence: validate the semantic relation—same address space and access, result pointee equals
  the array element—instead of incorrectly requiring whole pointer-type equality.

- Observation: nearby array-looking source shapes introduce different canonical owners.
  Evidence: a local `int[2]` retains `IRVar` plus generic-address-space `IRGetElementPtr`; a struct
  pointer retains `IRGetOffsetPtr` plus `IRFieldAddress` and struct keys/layout; a static constant
  array becomes `IRMakeArray` plus value `IRGetElement`. Established representative groupshared
  tests also require module globals, thread builtins, and barriers.
  Consequence: keep local allocation, structs/fields, SSA aggregate values, globals, shared memory,
  builtins, and barriers out of Slice 11. They are not alternate spellings of this input.

- Observation: a scalar `groupshared int` can retain a canonical rate-qualified `IRGlobalVar`, a
  helper load, and a kernel store without source `noinline`, but it is not yet a strong executable
  slice.
  Evidence: the linked probe retained that exact graph, while the existing real Slice 9 route shows
  libNVVM fully inlines ordinary helpers. The same-thread shared store/load can therefore collapse
  before PTX, just as the one-function form already collapses in Slang IR.
  Consequence: defer scalar shared storage until noinline/volatile or genuine cross-thread
  builtin/barrier semantics can keep shared-memory behavior observable. Do not claim one-thread
  scalar forwarding as shared-memory runtime evidence.

- Observation: a source pointer can name an explicit data-layout argument, but CUDA does not use
  that annotation to select aggregate stride.
  Evidence: `getTypeLayoutRuleNameForBuffer` returns `Natural` for CUDA before it examines a
  pointer's layout operand; the buffer-element legalization and established LLVM/CUDA emitter both
  consume that same target rule. For signed `i32`, the resulting natural stride is exactly LLVM's
  array stride.
  Consequence: do not add a downstream `DefaultBufferLayoutType` special case. The canonical
  representation boundary is the already-enforced absence of an explicit `IRArrayType` stride.

- Observation: the chosen ordinary source already compiles through explicit NVRTC.
  Evidence: `slangc.exe ... -emit-cuda-via-nvrtc` completed successfully on 2026-08-27.
  Consequence: preserve this source as the differential oracle without changing the default route.

## Decision Log

- Decision: Slice 11 is fixed, nonempty arrays of signed `i32` reached through existing device
  pointer kernel parameters.
  Rationale: this is the smallest aggregate/addressing capability that survives linking, has a
  stable raw CUDA ABI, and reuses existing signed-i32 values and scalar load/store. Local arrays add
  allocation/address-space semantics; structs add field identity/layout; shared memory adds globals
  and core CUDA execution.
  Date/Author: 2026-08-27, Codex.
  Revisit when: Slice 11 is complete and the next bounded type/memory source shape is probed.

- Decision: append one coherent V2 prefix containing `getArrayType` and
  `emitArrayElementPointer`.
  Rationale: the provider must construct the canonical LLVM array type used in the function ABI,
  and typed GEP must derive its element result from that array pointee. Reusing pointer-offset GEP
  would return another array pointer and would erase the distinct `IRGetElementPtr` semantic.
  Date/Author: 2026-08-27, Codex.
  Revisit when: a future opaque-pointer provider ABI supplies explicit source/result types.

- Decision: provider array counts use a nonzero `uint32_t`, while Slang accepts any exact fixed
  count representable by that contract rather than only the test fixture's count four.
  Rationale: a 32-bit nonzero bound prevents impossible/overflow-prone GPU object sizes at the ABI
  edge without hardcoding a particular array. Unsized and empty arrays are different canonical
  shapes and remain outside the slice.
  Date/Author: 2026-08-27, Codex.
  Revisit when: a demonstrated Slang/CUDA use requires a wider fixed count or flexible array.

- Decision: emit ordinary, non-`inbounds` array GEP.
  Rationale: Slang subscript supplies an index but this slice does not prove an LLVM provenance or
  bounds contract. Runtime uses valid endpoints, while out-of-bounds behavior remains outside the
  supported guarantee.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an upstream invariant establishes LLVM-compatible inbounds provenance.

## Outcomes and Retrospective

Slice 11 now accepts the selected canonical fixed-device-array shape end to end. The fake graph
proved one shared `[4 x i32]` provider type, two array-pointer parameters, the shared signed-i32
index, both base/index GEP relations, the source-element load, and destination-element store. The
real LLVM 14 provider verified exactly two ordinary, non-`inbounds` array GEPs with leading zero,
one aligned i32 load/store pair, and one kernel annotation. Its invalid-call matrix left exactly
those valid instructions after rejecting type, count, context, insertion-point, ownership, and
dominance errors.

The formatted Release provider and Debug host builds passed. The focused prefix passed 76/76.
Direct NVVM and NVRTC both exposed `[64, 64, 32]` and entry-scoped global i32 load/store behavior;
CUDA 12.9 `ptxas` accepted both outputs. Both routes copied indices 0 and 3 on the RTX 5090 while
preserving every neighboring destination sentinel. The preservation matrix passed parser 1/1,
routing/hash 2/2, unsupported file 1/1, sampler 3/3, NVRTC pass-through 2/2, and runtime dispatch
1/1. The final provider exports only the V1/V2 getters, depends on `KERNEL32.dll` plus delay-loaded
`SHELL32.dll`/`ole32.dll`, and has no process-visible LLVM DLL dependency.

The required helper audit retained only canonical classifiers, provider-type caching, and the
typed array-element operation. It removed the redundant `getArrayStride()` check because exact
two-operand `IRArrayType` already owns that condition. A tentative default-pointer-layout guard
was also rejected after tracing the CUDA producer: CUDA selects natural layout before inspecting
the pointer annotation, so the real boundary is the canonical array's absence of an explicit
stride. No custom equivalence, syntax reconstruction, target-specific downstream repair, or
fallback remains.

## Context and Current Pipeline

`source/slang/slang-emit.cpp` links and optimizes the selected program, preserves raw CUDA
signatures for explicit direct NVVM, and calls `validateNVVMSupportedIR` before builder discovery.
`source/slang/slang-emit-nvvm.cpp` walks the finite direct-call closure, validates each function in
dominance order, negotiates the maximum required `NVVMIRCapability`, declares all functions and
parameters, maps canonical IR values to provider handles, emits bodies, and serializes once.

Slice 10 accepts signed-i32/device-`Ptr<int>` parameters and `IRGetOffsetPtr`. Its
`_asSupportedDevicePointerType`, `_validatePointerValue`, `_validateI32Value`, and canonical value
map are the existing sources of truth. Slice 11 must add an exact array-pointer classifier and an
exact relation check for `IRGetElementPtr`. Existing scalar load/store validation remains the sole
owner of element-pointer read/write access.

The private V2 ABI now ends at the 264-byte 64-bit Slice 11 prefix with `getArrayType` and
`emitArrayElementPointer`; the 248-byte Slice 10 minimum remains frozen. The host rejects partial
capability blocks, accepts/clamps future-larger tables, clears provider outputs on every failed
wrapper call, and hashes stable capability identity bits. The LLVM 14.0.6 provider owns a
typed-pointer `LLVMContext`, module, and `IRBuilder` per module and validates
context/module/function/dominance before every mutation.

## Scope and Non-Goals

In scope:

- exact `IRArrayType` with a nonzero `uint32_t` count, signed-i32 element, and no explicit custom
  stride;
- device `Ptr<int[N]>` entry parameters with read or read-write access;
- exact two-operand `IRGetElementPtr` with an available signed-i32 index;
- device `Ptr<int>` results preserving base address space and access;
- one coherent append-only provider array-type/addressing capability;
- fake, verified LLVM, differential PTX, `ptxas`, and CUDA runtime evidence.

Explicitly out of scope:

- unsized, empty, nested, vector, matrix, struct, tuple, or non-i32 arrays;
- array values, `IRMakeArray`, `IRGetElement`, aggregate load/store/copy, or array returns;
- local `IRVar`/alloca, globals, constants in memory, shared/local/constant/generic address spaces;
- `IRFieldAddress`, struct layout/keys, pointer-to-array helpers, or aggregate kernel values;
- unsigned/wider indices, bounds checks, `inbounds`, allocation provenance, or sanitizers;
- thread/block builtins, barriers, atomics, shared memory, resources, or libdevice.

## Architecture and Invariants

Linked Slang IR remains the semantic source of truth. An accepted array parameter is an exact
device pointer whose value type is an exact fixed `IRArrayType` of signed `i32`, whose count is a
nonzero `IRIntLit` fitting `uint32_t`, and which has no explicit stride operand. CUDA's canonical
buffer-element lowering uses natural layout for pointer pointees regardless of a source pointer's
layout annotation, so the array's lack of an explicit strideâ€”not the pointer's retained layout
operandâ€”is the storage-shape invariant. Preflight accepts an `IRGetElementPtr` only when it has
exactly two operands, its base is an available accepted array pointer, its index is an available
signed-i32 value, and its result is an accepted scalar device pointer with exactly the base address
space and access. It does not reconstruct syntax or infer layout from names.

Capability selection remains monotonic. `ScalarArrayAddressing` is terminal after
`ScalarPointerArithmetic`. An exact Slice 10 provider continues to compile every earlier shape and
rejects an array program as E52016 after discovery but before builder-module creation. The new
`SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE` is 264 bytes on the 64-bit build; the 248-byte
Slice 10 minimum and every older minimum remain frozen. A table ending inside either new pointer or
containing a null new member is malformed.

The provider is general over valid loadable element types and NVVM pointer address spaces, while
the Slang boundary owns the fixed-i32/device policy. `getArrayType` validates a live module,
non-null output, same-context element type, `ArrayType::isValidElementType`,
`PointerType::isLoadableOrStorableType`, `isSized()`, and nonzero count before returning LLVM
`[N x element]`. `emitArrayElementPointer` validates a live module, output, current unterminated
insertion block, same-module typed non-opaque pointer in a declared NVVM address space whose
pointee is a sized LLVM array, scalar integer index, and value availability. It then emits one
non-`inbounds`
`CreateGEP(arrayType, base, {i32 0, index})`. All validation precedes the sole mutation.

## Interfaces and Dependencies

Append after Slice 10 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetArrayType_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle_1* outType);

typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitArrayElementPointer_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 baseArrayPointer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1* outPointer);
```

Add host methods `supportsScalarArrayAddressing()`, `getArrayType(...)`, and
`emitArrayElementPointer(...)`; identity `scalar-array-addressing=0|1`; and terminal
`NVVMIRCapability::ScalarArrayAddressing`. No public Slang API changes. The implementation retains
the optional statically linked LLVM 14 provider and established libNVVM/NVRTC, CUDA 12.9 `ptxas`,
and CUDA-driver gates.

## Milestones

1. Freeze the two-member provider suffix, minimum-size macro, C compile probes, host coherent-prefix
   validation, identity, wrappers, provider getter, and exact Slice 10 compatibility. Test every
   partial/null/larger/output-sanitization case before changing Slang emission.

2. Implement provider array type creation and element GEP with complete pre-mutation validation.
   Add a verified `[4 x i32]` AS1 copy kernel and invalid/no-mutation coverage for null/foreign/type,
   insertion-point, cross-function, and non-dominating shapes.

3. Extend Slang preflight, parameter type construction, capability gating, and body emission for
   exact fixed-i32 array pointers and `IRGetElementPtr`. Cache lowered array/pointer types by the
   canonical `IRArrayType`; preserve result handles in the existing value map. Do not add a generic
   aggregate lowering framework or repair alternative shapes.

4. Extend the fake builder to model integer/array/pointer type identity and element-pointer result
   identity. Prove exact function parameter types/counts and base/index/result/load/store topology,
   plus absence of pointer-offset/arithmetic/control/function operations.

5. Add exact old-provider gating, unsigned/non-i32/nested/local/struct boundaries, NVVM/NVRTC PTX
   differential, both `ptxas` routes, and endpoint runtime sentinel coverage.

6. Run the complete formatted validation matrix and binary inspection. Perform the required helper
   and input-shape audit, distill stable behavior into `docs/design/nvvm-backend.md` and the
   capability ledger, keep this plan untracked, and commit exactly the tracked Slice 11 files as
   `slice 11`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. All CMake builds and tests run outside the
sandbox as required by `AGENTS.md`.

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm
cmake.exe --build build --config Debug --target slang-test

$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Re-run established preservation regressions:

```text
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/coverageCudaRuntimeDispatch
```

Acceptance requires:

- exact final linked array-pointer signature and two `IRGetElementPtr` producer/consumer chains;
- fake topology checks type count/element/base/access/index/result/load/store identities, not only
  call counts;
- verified provider assembly contains `[4 x i32] addrspace(1)*`, two ordinary array GEPs with
  leading zero, one aligned i32 load/store, no `inbounds`, and one kernel annotation;
- invalid calls and partial prefixes clear outputs and cause no partial LLVM mutation;
- an exact Slice 10 provider retains all prior programs and gates only the new shape before module
  creation;
- direct NVVM and NVRTC expose `[64,64,32]`, agree on entry-scoped global load/store, and both
  assemble;
- both routes copy indices 0 and 3 and preserve all other destination sentinels;
- adjacent aggregate/address-space shapes remain deterministic unsupported boundaries;
- the final NVVM prefix and preservation matrix pass after formatting;
- the provider exports only the V1/V2 getters, has no process-visible LLVM DLL dependency, pinned
  clang-format makes no changes, and `git diff --check` passes.

## Failure and Recovery

Probes, incremental builds, focused tests, formatter diff checks, and binary inspection are safe to
repeat. If the chosen source stops retaining exact `IRGetElementPtr`, fix or reassess the producer
contract; do not accept a byte-offset or syntax fallback. If LLVM verification fails, fix provider
type/GEP validation before serialization. Failed-output and no-mutation tests separate ABI wrapper
failures from provider construction and libNVVM compilation.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
sources. Remove temporary Slice 11 probes with `apply_patch` before committing. The direct route
remains experimental and removable without affecting default NVRTC dispatch.

## Artifacts and Hand-Off

The durable architecture and complete validation evidence are recorded in
`docs/design/nvvm-backend.md`; all eight exact test rows are recorded in
`docs/design/nvvm-backend-capability-ledger.md`. The final tracked diff is formatted and clean, and
all temporary Slice 11 probes have been removed with `apply_patch`. The tracked work is the
`slice 11` commit; keep this and prior ExecPlans untracked.
