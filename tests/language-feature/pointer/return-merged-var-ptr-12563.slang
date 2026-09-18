//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// Positive companion to return-merged-conflict-12563.slang. The `int*` local
// `result` is assigned along two paths but from pointers in the *same* concrete
// storage class (both Workgroup: `&gA` and `&gB`), then returned once. `result`
// is not SSA-promoted, so it reaches specializeAddressSpace as a pointer-holding
// local (`Ptr(Ptr(int))`) whose inner pointer keeps the pre-specialization default
// while the stores are Workgroup — the OpStore/OpLoad types disagree until the
// held address space is reconciled from the stores. Before the fix even this
// consistent case produced invalid SPIR-V; reconcileHeldPointerAddressSpace now
// rewrites the variable's inner pointer (and its loads) to the shared Workgroup
// class so it validates.
//
// Compiles with `-target spirv` (SPIR-V disassembly) so the FileCheck lines have
// text to match; CI's `SLANG_RUN_SPIRV_VALIDATION=1` validates the reconciled
// types.

RWStructuredBuffer<int> outputBuffer;

groupshared int gA;
groupshared int gB;

[noinline]
int* pick(bool c)
{
    int* result;
    if (c)
        result = &gA;
    else
        result = &gB;
    return result;
}

// CHECK-DAG: OpEntryPoint
// CHECK-DAG: OpFunctionCall
// CHECK-NOT: error
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gA = int(tid.x);
    gB = int(tid.y);
    int* r = pick(tid.x == 0);
    outputBuffer[0] = *r;
}
