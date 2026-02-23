// Groupshared variables with interface types are not supported because
// existential type lowering (type tag + AnyValue packing) is incompatible
// with workgroup-shared memory allocation, which requires fixed layout
// known at compile time.
//
// Currently this causes an internal compiler error (ICE 99999) during
// code emission because the existential lowering passes do not handle
// groupshared-rate variables. Once a proper diagnostic is added during
// semantic checking, these tests should be enabled.
//
// Scenarios:
//   1. Direct groupshared interface variable
//   2. Groupshared array of interface-typed values
//   3. Groupshared struct containing interface field (currently compiles
//      but correctness is unverified; may need separate validation)

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

// CHK1: error
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

// CHK2: error
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
