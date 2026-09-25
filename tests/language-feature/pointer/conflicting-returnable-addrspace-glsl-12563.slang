//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -stage compute -entry computeMain

// GLSL companion to conflicting-returnable-addrspace-metal-12563.slang for
// shader-slang/slang#12563. On GLSL a pointer's address space is part of its type, so a function
// that returns pointers in two different address spaces is a type mismatch caught in semantic
// checking (E30019) — before the shared address-space specialization pass. This is the positive
// demonstration that the E58005 / held-pointer conflict path is unreachable on GLSL, which is why
// `reconcileHeldPointerAddressSpace` and the slot pre-pass are gated SPIR-V-only: the conflict
// never reaches the pass. `p` is a device pointer; the address of the `groupshared` `gs` is a
// threadgroup pointer, and one result type cannot be both.
//
// non-exhaustive: GLSL additionally reports E36107 (raw-pointer features are not all available on
// the target); the point pinned here is the earlier E30019 type error, so only it is annotated.

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
