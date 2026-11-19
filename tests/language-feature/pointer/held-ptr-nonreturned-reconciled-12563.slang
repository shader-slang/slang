//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// Positive companion to held-ptr-nonreturned-conflict-12563.slang for shader-slang/slang#12563.
// `p` is a non-returned `Ptr(Ptr(int))` local written a Workgroup pointer on both paths (from two
// `[noinline]` callees returning `&gA` / `&gB`), then only dereferenced. The stores come from calls,
// so the storage class only becomes concrete after the callees settle; `reconcileHeldPointerAddressSpace`
// then rewrites `p`'s inner pointer and its loads to the shared Workgroup class so the OpStore/OpLoad
// types agree and the module validates.
//
// The two CHECK-DAG lines pin the *reconciled* class directly: `p`'s slot is a Function pointer whose
// pointee is a Workgroup int pointer (`OpTypePointer Function %_ptr_Workgroup_int`), and the load off
// `p` yields that Workgroup pointer. That pointer-to-pointer type exists only because `p` was
// reconciled to Workgroup — it is not produced by the groupshared source objects on their own — so a
// regression that leaves `p`'s slot in the default class fails this test even with SPIR-V validation
// off (CI additionally validates the reconciled types via SLANG_RUN_SPIRV_VALIDATION=1).

RWStructuredBuffer<int> buf;

groupshared int gA;
groupshared int gB;

[noinline]
int* pickA()
{
    return &gA; // Workgroup
}

[noinline]
int* pickB()
{
    return &gB; // Workgroup
}

// CHECK-DAG: OpTypePointer Function %_ptr_Workgroup_int
// CHECK-DAG: OpLoad %_ptr_Workgroup_int %p
// CHECK-NOT: error

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gA = 1;
    gB = 2;
    int* p;
    if (tid.x == 0)
        p = pickA();
    else
        p = pickB();
    buf[0] = *p; // dereferenced, not returned
}
