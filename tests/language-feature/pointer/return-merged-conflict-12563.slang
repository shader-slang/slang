//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// Companion to conflicting-returnable-addrspace-12563.slang, which has two
// returns in different storage classes. Here the conflict is merged *before* a
// single return: the `int*` local `result` is assigned a `groupshared` pointer
// ('Workgroup') on one path and a `RWStructuredBuffer` element pointer
// ('StorageBuffer') on the other, then returned once. `result` is not
// SSA-promoted here — it survives to specializeAddressSpace as a pointer-holding
// local (a `Ptr(Ptr(int))` variable) whose two `store`s carry the two classes.
// A pointer variable can hold pointers of only one storage class, so the held
// pointer would need both at once. Before the fix nothing reconciled the held
// address space from the stores, so the OpStore/OpLoad pointer types disagreed
// (invalid SPIR-V / internal error) with no diagnostic. reconcileHeldPointer-
// AddressSpace now records the same conflict reported for divergent returns
// (E58005).
//
// (Metal and WGSL reject this earlier, in the type checker: a pointer's address
// space is part of its type there, so the two assignments are a type mismatch,
// E30019, before this pass — see conflicting-returnable-addrspace-metal-12563.slang.)

RWStructuredBuffer<int> buf;

groupshared int gShared;

[noinline]
int* pick(bool c, uint i)
{
    int* result;
    if (c)
        result = &gShared;
    else
        result = &buf[i];
    return result;
}
//CHECK: 58005
//CHECK: single address space

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gShared = 1;
    int* r = pick(tid.x == 0, tid.x);
    buf[0] = *r;
}
