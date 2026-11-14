// Regression test for shader-slang/slang#12785 (coherent memory and the repeatable-read-only gate).
//
// The CSE added for #12785 commons a dominated identical call only when the callee performs a
// "repeatable read-only access" -- it reads exclusively IMMUTABLE (read-only SRV) memory. A wrapper
// around a `globallycoherent` (RW) buffer reads MUTABLE memory, so it is not repeatable and its
// calls must not be commoned. Conversely, a bare `globallycoherent` LOAD sitting between two calls
// to an ordinary read-only callee is a read, not a write, and (per the SPIR-V/Vulkan memory model)
// carries no acquire/fence effect on other memory -- so it does NOT block reuse; only an actual
// write/atomic/barrier does (covered by gh-12785-negative.slang).

//TEST:SIMPLE(filecheck=CALLEE): -target cuda -entry mainCoherentCallee -stage compute
//TEST:SIMPLE(filecheck=INTERVENE): -target cuda -entry mainInterveningCoherent -stage compute

globallycoherent RWStructuredBuffer<uint> coherentData;
StructuredBuffer<uint> plainData;
globallycoherent RWStructuredBuffer<uint> syncData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadCoherent(uint index)
{
    return coherentData[index];
}

[noinline]
uint loadPlain(uint index)
{
    return plainData[index];
}

// A coherent-reading callee reads mutable memory, so it is not a repeatable read-only access and
// its two calls stay distinct.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainCoherentCallee(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadCoherent(index);
    uint b = loadCoherent(index);
    outputData[index] = a + b;
}
// CALLEE-COUNT-2: loadCoherent_0(index
// CALLEE-NOT: loadCoherent_0(index

// An intervening bare coherent LOAD is a read with no acquire/fence effect on other memory, so it
// does not block reuse of the ordinary read-only callee: the two calls collapse to one.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainInterveningCoherent(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadPlain(index);
    uint s = syncData[index]; // bare coherent load between the calls: not a write, no acquire
    uint b = loadPlain(index);
    outputData[index] = a + b + s;
}
// INTERVENE-COUNT-1: loadPlain_0(index
// INTERVENE-NOT: loadPlain_0(index
