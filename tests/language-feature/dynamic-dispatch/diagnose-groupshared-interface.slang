// Direct groupshared variables with interface types.
// Verifies that the type flow analysis properly handles loads from
// interface-typed global variables by enumerating conforming types,
// enabling dynamic dispatch without requiring struct wrapping.
//
// See https://github.com/shader-slang/slang/issues/10144
//
// The struct-wrapping workaround is validated by
// groupshared-struct-with-interface.slang.

// --- Scenario 1: Direct groupshared interface variable ---
//TEST:SIMPLE(filecheck=SPV1):-target spirv-asm -stage compute -entry scenario1
//TEST:SIMPLE(filecheck=HLSL1):-target hlsl -stage compute -entry scenario1

interface IValue
{
    float get();
}

struct ScalarValue : IValue
{
    float v;
    float get() { return v; }
}

struct DoubleValue : IValue
{
    float v;
    float get() { return v * 2.0; }
}

groupshared IValue sharedVal;

RWStructuredBuffer<float> outputBuffer;

// SPV1: OpEntryPoint
// HLSL1: groupshared
[numthreads(64, 1, 1)]
void scenario1(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
        sharedVal = ScalarValue(42.0);
    else if (tid.x == 1)
        sharedVal = DoubleValue(10.0);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = sharedVal.get();
}

// --- Scenario 2: Groupshared array of interface-typed values ---
//TEST:SIMPLE(filecheck=SPV2):-target spirv-asm -stage compute -entry scenario2
//TEST:SIMPLE(filecheck=HLSL2):-target hlsl -stage compute -entry scenario2

groupshared IValue sharedArray[4];

// SPV2: OpEntryPoint
// HLSL2: groupshared
[numthreads(4, 1, 1)]
void scenario2(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 2)
        sharedArray[tid.x] = ScalarValue(float(tid.x));
    else
        sharedArray[tid.x] = DoubleValue(float(tid.x));
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = sharedArray[tid.x].get();
}
