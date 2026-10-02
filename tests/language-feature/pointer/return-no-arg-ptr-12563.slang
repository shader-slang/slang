//TEST:SIMPLE(filecheck=WORKGROUP):-target spirv -entry computeMainWorkgroup -stage compute
//TEST:SIMPLE(filecheck=STORAGE):-target spirv -entry computeMainStorage -stage compute

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// A pointer-returning function does not need a pointer *argument*: it can pick up
// a concrete address space entirely from what it returns. Argument specialization
// and result reconciliation are independent concerns, so a callee with no pointer
// argument (`pickShared`) or only a scalar argument (`pickBuf`) still has its
// result address space reconciled onto the call. Before the fix,
// specializeAddressSpace bailed out of the call on `!hasSpecializableArg` before
// recording the call result, so the OpFunctionCall result type kept its default
// while the callee's return type became Workgroup/StorageBuffer — an
// OpFunctionCall result-type mismatch (invalid SPIR-V) for the simplest case the
// feature exists to support.
//
// These compile with `-target spirv` (SPIR-V disassembly) so the
// OpFunctionCall/OpEntryPoint FileCheck lines have text to match; SPIR-V
// validation of the reconciled result type is enabled independently by CI's
// `SLANG_RUN_SPIRV_VALIDATION=1` (see shouldRunSPIRVValidation), which catches the
// pre-fix mismatch. `[noinline]` keeps the pointer crossing a real call boundary.

RWStructuredBuffer<int> outputBuffer;

groupshared int gShared;

// Zero pointer arguments; returns a Workgroup pointer.
[noinline]
int* pickShared()
{
    return &gShared;
}

// WORKGROUP-DAG: OpEntryPoint
// WORKGROUP-DAG: OpFunctionCall
// WORKGROUP-NOT: error
[numthreads(1, 1, 1)]
void computeMainWorkgroup(uint3 tid: SV_DispatchThreadID)
{
    gShared = int(tid.x);
    int* got = pickShared();
    outputBuffer[0] = *got;
}

// Only a scalar argument; returns a StorageBuffer pointer.
[noinline]
int* pickBuf(uint i)
{
    return &outputBuffer[i];
}

// STORAGE-DAG: OpEntryPoint
// STORAGE-DAG: OpFunctionCall
// STORAGE-NOT: error
[numthreads(1, 1, 1)]
void computeMainStorage(uint3 tid: SV_DispatchThreadID)
{
    int* got = pickBuf(tid.x);
    *got = int(tid.x);
}
