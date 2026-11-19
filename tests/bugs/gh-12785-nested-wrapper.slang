// Regression test for shader-slang/slang#12785 (transitive eligibility through a nested call).
//
// `calleeIsRepeatableReadOnly` recurses into the callee's own calls: an outer `[noinline]` wrapper
// that itself calls an inner `[noinline]` wrapper is repeatable-read-only only if the inner one is.
// This exercises the recursion branch (`if (auto call = as<IRCall>(inst)) { if
// (!calleeIsRepeatableReadOnly(call->getCallee(), visiting)) ... }`), which no other test covers.
//
// `outerPlain` calls `innerPlain`, which reads an immutable `StructuredBuffer` -> repeatable -> the
// two dominated `outerPlain` calls are commoned. `outerRW` calls `innerRW`, which reads a mutable
// `RWStructuredBuffer` -> NOT repeatable -> the recursion propagates ineligibility up and both
// `outerRW` calls survive. If the recursion were broken to return `true` for the mutable-reading
// inner callee, the two `outerRW` calls would be wrongly commoned across an unmodelled write.

//TEST:SIMPLE(filecheck=PLAIN): -target hlsl -entry mainPlain -stage compute
//TEST:SIMPLE(filecheck=RW): -target hlsl -entry mainRW -stage compute

StructuredBuffer<uint> roData;
RWStructuredBuffer<uint> rwData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint innerPlain(uint index)
{
    return roData[index];
}

[noinline]
uint innerRW(uint index)
{
    return rwData[index];
}

[noinline]
uint outerPlain(uint index)
{
    return innerPlain(index) + 1;
}

[noinline]
uint outerRW(uint index)
{
    return innerRW(index) + 1;
}

// Inner reads immutable memory: outer is repeatable through the recursion, so the calls common.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainPlain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = outerPlain(i) + outerPlain(i);
}
// PLAIN-COUNT-1: outerPlain_0(i
// PLAIN-NOT: outerPlain_0(i

// Inner reads mutable memory: the recursion makes outer ineligible, so both calls survive.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainRW(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = outerRW(i) + outerRW(i);
}
// RW-COUNT-2: outerRW_0(i
// RW-NOT: outerRW_0(i
