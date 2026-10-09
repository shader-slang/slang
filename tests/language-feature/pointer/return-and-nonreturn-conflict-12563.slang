//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// A single local pointer slot cannot hold pointers in two different concrete address spaces.
// Two distinct diagnostics cover that defect, and which one applies is decided per slot, not per
// function: a slot whose merged value is returned is a return conflict (E58005, which names the
// disagreeing return), while a slot that is only dereferenced is the general inconsistent-slot
// conflict (E58003). `mixed` has one of each -- `p` is stored a StorageBuffer element pointer and
// a physical Device pointer and only dereferenced, and `r` is stored a Workgroup and a
// StorageBuffer pointer and returned -- so one compilation of one function raises both.
//
// Neither local is SSA-promoted, so both reach specializeAddressSpace as pointer-holding locals
// whose two stores carry the two classes.

RWStructuredBuffer<int> buf;

groupshared int gShared;

[noinline]
int* mixed(bool c, uint i)
{
    int* p = &buf[0];             // StorageBuffer
    if (c)
        p = (int*)(0x1000ull);    // physical Device
    buf[1] = *p;                  // dereferenced, not returned

    int* r;
    if (c)
        r = &gShared;             // Workgroup
    else
        r = &buf[i];              // StorageBuffer
    return r;
}
//CHECK: E58003
//CHECK: use of pointer with inconsistent address space
//CHECK: E58005
//CHECK: single address space

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gShared = 1;
    int* q = mixed(tid.x == 0, tid.x);
    buf[0] = *q;
}
