//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Companion to pointer-address-space-conflict.slang, covering the case where the *physical* value
// merged into a slot is a function parameter rather than an integer-to-pointer cast. Here `pick`
// is not inlined, so its `int* ext` parameter reaches the address-space specialization pass as a
// real function parameter with its declared (physical Device) address space. Inside `pick`, the
// slot `p` is fed both a descriptor-backed StorageBuffer element pointer (&buf[0]) and that
// physical parameter, so the pass must diagnose the inconsistency. This guards the distinction the
// pass draws when resolving a stored value's address space: a *function* parameter carries an
// authoritative address space and must be honored, whereas an unresolved *phi* is treated as
// unknown; conflating the two would let this conflict compile silently into an invalid OpStore.
// non-exhaustive: the diagnostic is raised during SPIR-V legalization, after source locations have
// been dropped, so it carries no line to annotate.

//CHECK: use of pointer with inconsistent address space

RWStructuredBuffer<int> buf;
RWStructuredBuffer<int> outp;

[noinline]
int pick(int* ext, bool c)
{
    int* p = &buf[0];
    if (c)
        p = ext;
    return *p;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outp[0] = pick((int*)(0x1000ull), tid.x == 0);
}
