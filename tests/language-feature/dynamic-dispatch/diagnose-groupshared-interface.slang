// Direct groupshared variables with interface types currently trigger an
// ICE because the existential lowering passes do not handle groupshared-rate
// variables. The existential layout is known after compilation, so this may
// be a fixable limitation rather than a fundamental incompatibility.
// Disabled until the fix lands. See https://github.com/shader-slang/slang/issues/10144
//
// The struct-wrapping workaround is validated by
// groupshared-struct-with-interface.slang.

// --- Scenario 1: Direct groupshared interface variable ---
//DISABLE_TEST:SIMPLE(filecheck=CHK1): -target spirv -stage compute -entry scenario1
//DISABLE_TEST:SIMPLE(filecheck=CHK1): -target hlsl -stage compute -entry scenario1

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

[numthreads(64, 1, 1)]
void scenario1(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
        sharedVal = ScalarValue(42.0);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = sharedVal.get();
}

// --- Scenario 2: Groupshared array of interface-typed values ---
//DISABLE_TEST:SIMPLE(filecheck=CHK2): -target spirv -stage compute -entry scenario2
//DISABLE_TEST:SIMPLE(filecheck=CHK2): -target hlsl -stage compute -entry scenario2

groupshared IValue sharedArray[4];

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
