//TEST:SIMPLE(filecheck=GROUPSHARED):-target spirv -entry computeMainGroupshared -stage compute
//TEST:SIMPLE(filecheck=NESTED):-target spirv -entry computeMainNested -stage compute
//TEST:SIMPLE(filecheck=FORCEINLINE):-target spirv -entry computeMainForceInline -stage compute

// Regression test for shader-slang/slang#12498.
//
// A `[noinline]` helper that returns a pointer whose pointee lives in a concrete
// address space that outlives the call (here Workgroup, supplied by the caller's
// `groupshared` global) must compile to valid SPIR-V: the OpFunctionCall result
// type has to match the specialized callee's return type. `[noinline]` keeps the
// pointer crossing a real call boundary. These compile with `-target spirv`
// (SPIR-V disassembly) so the OpFunctionCall/OpEntryPoint FileCheck lines have
// text to match; SPIR-V validation of that reconciled result type is enabled
// independently by CI's `SLANG_RUN_SPIRV_VALIDATION=1` (see shouldRunSPIRVValidation),
// which is target-independent.
//
// The helpers below use `if (cond) return p; return p;` — both arms return the
// same pointer deliberately, so the pointer is returned along more than one
// control-flow path (as in the original #12498 repro) and its address space must
// be reconciled at the merge, not just on a single straight-line return.

RWStructuredBuffer<int> outputBuffer;

groupshared int gShared;

// `pickShared` is address-space-agnostic — it returns whatever pointer it is
// given. The Workgroup address space enters only because `computeMainGroupshared`
// passes `&gShared`; after specialization the returned pointer is Workgroup, so
// the call result must be reconciled to Workgroup rather than left at the default.
[noinline]
int* pickShared(int* p, bool cond)
{
    if (cond)
        return p;
    return p;
}

// GROUPSHARED-DAG: OpEntryPoint
// GROUPSHARED-DAG: OpFunctionCall
// GROUPSHARED-NOT: error
[numthreads(1, 1, 1)]
void computeMainGroupshared(uint3 tid: SV_DispatchThreadID)
{
    gShared = int(tid.x);
    int* got = pickShared(&gShared, tid.x > 0);
    outputBuffer[0] = *got;
}

// Two-level nesting: the outer helper returns the pointer produced by an inner
// helper. This exercises the ordering in which a freshly specialized callee is
// settled before its caller's call result is recorded.
[noinline]
int* pickInner(int* p, bool cond)
{
    if (cond)
        return p;
    return p;
}

[noinline]
int* pickOuter(int* p, bool cond)
{
    return pickInner(p, cond);
}

// NESTED-DAG: OpEntryPoint
// NESTED-DAG: OpFunctionCall
// NESTED-NOT: error
[numthreads(1, 1, 1)]
void computeMainNested(uint3 tid: SV_DispatchThreadID)
{
    gShared = int(tid.x);
    int* got = pickOuter(&gShared, tid.x > 0);
    outputBuffer[0] = *got;
}

// The E58004 remedy the diagnostic advertises: `[ForceInline]` a helper that
// returns a pointer to a caller's function-local. Force-inlining runs before the
// address-space pass, so no pointer crosses a call boundary and no `Function`-
// storage-class result survives to be diagnosed — the module must compile
// cleanly with no error (in particular, no E58004).
[ForceInline]
int* pickLocal(int* p, bool cond)
{
    if (cond)
        return p;
    return p;
}

// FORCEINLINE: OpEntryPoint
// FORCEINLINE-NOT: error
// FORCEINLINE-NOT: 58004
[numthreads(1, 1, 1)]
void computeMainForceInline(uint3 tid: SV_DispatchThreadID)
{
    int v = int(tid.x);
    int* got = pickLocal(&v, tid.x > 0);
    outputBuffer[0] = *got;
}
