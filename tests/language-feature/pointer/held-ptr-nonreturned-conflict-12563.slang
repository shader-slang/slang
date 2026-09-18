//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563: a held pointer written two different concrete
// storage classes but NOT returned must still be diagnosed, not left as invalid SPIR-V.
//
// `p` is a `Ptr(Ptr(int))` local written a Workgroup pointer (from `pickShared`) on one path and a
// StorageBuffer pointer (from `pickBuf`) on the other, then only dereferenced. The two stores come
// from calls, so the storage classes only become concrete after the callees settle during the
// dataflow — the slot pre-pass `reconcilePointerSlotWithStoredValue` resolves stored values with
// `getStoredValueAddrSpace`, which has no call case and sees `Generic`, so it cannot flag this.
// `reconcileHeldPointerAddressSpace` runs during the drain (after the calls resolve) and detects
// the conflict; because `p` is not returned it is the general inconsistent-slot conflict, reported
// as E58003. Before the fix this branch dropped a non-returned conflict silently and emitted
// invalid SPIR-V (disagreeing OpStore/OpLoad pointer types) with no diagnostic on any path.

RWStructuredBuffer<int> buf;

groupshared int gShared;

[noinline]
int* pickShared()
{
    return &gShared; // Workgroup
}

[noinline]
int* pickBuf(uint i)
{
    return &buf[i]; // StorageBuffer
}
//CHECK: E58003
//CHECK: use of pointer with inconsistent address space

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gShared = 1;
    int* p;
    if (tid.x == 0)
        p = pickShared();
    else
        p = pickBuf(tid.x);
    buf[0] = *p; // dereferenced, not returned
}
