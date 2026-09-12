//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// A single pointer local cannot hold values from two different concrete address spaces. Here a
// descriptor-backed StorageBuffer element pointer (&buf[0]) and a physical Device pointer built
// from an integer address are stored into the same int* slot across control flow. The address-
// space specialization pass diagnoses this rather than silently choosing one, matching the
// existing phi-argument behavior (shader-slang/slang#12581). non-exhaustive: the diagnostic is
// raised during SPIR-V legalization, after source locations have been dropped, so it carries no
// line to annotate.

//CHECK: use of pointer with inconsistent address space

RWStructuredBuffer<int> buf;
RWStructuredBuffer<int> outp;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = &buf[0];
    if (tid.x == 0)
        p = (int*)(0x1000ull);
    outp[0] = *p;
}
