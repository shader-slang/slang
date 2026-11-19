//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain

// Companion to conflicting-returnable-addrspace-12563.slang for
// shader-slang/slang#12563. On Metal (and WGSL) a pointer's address space is
// part of its type, so a function that returns pointers in two different address
// spaces is a type mismatch caught in semantic checking (E30019) — before the
// shared address-space specialization pass that reports E58005 for SPIR-V. This
// pins that boundary: the same conflict that yields E58005 on SPIR-V is rejected
// earlier here, so E58005 is unreachable on Metal not because pointers are
// unavailable (they are — see tests/metal/metal-pointer-params.slang) but
// because the disagreement is a type error. `p` is a device pointer; the address
// of the `groupshared` `gs` is a threadgroup pointer, and one result type cannot
// be both.

RWStructuredBuffer<int> output;
groupshared int gs;

[noinline]
int* pick(bool c, uniform int* p)
{
    if (c)
        return __getAddress(gs);
    return p;
}
//CHECK: 30019
//CHECK: AddressSpace.GroupShared

[numthreads(1, 1, 1)]
void computeMain(uniform int* g, uint3 tid: SV_DispatchThreadID)
{
    gs = 1;
    output[tid.x] = *pick(tid.x == 0, g);
}
