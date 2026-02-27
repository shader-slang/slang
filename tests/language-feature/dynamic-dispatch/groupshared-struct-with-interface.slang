// Groupshared memory containing a struct with an interface-typed field.
//
// Unlike a bare `groupshared IFoo`, wrapping the interface in a struct
// allows the existential lowering to produce a fixed-size tuple (type tag
// + AnyValue) inside the struct before the groupshared allocation is
// finalized. This test verifies that dynamic dispatch through a
// groupshared struct's interface member works correctly at runtime.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type

//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ITransform
{
    float apply(float x);
}

struct Identity : ITransform
{
    float apply(float x) { return x; }
}

struct Doubler : ITransform
{
    float scale;
    float apply(float x) { return x * scale; }
}

struct SharedState
{
    ITransform transform;
    float baseValue;
}

groupshared SharedState state;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    state.baseValue = 10.0;
    state.transform = Identity();
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[0] = state.transform.apply(state.baseValue);
    // CHECK: 10.0
    // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code

    state.transform = Doubler(3.0);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[1] = state.transform.apply(state.baseValue);
    // CHECK: 30.0

    state.baseValue = 5.0;
    state.transform = Identity();
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[2] = state.transform.apply(state.baseValue);
    // CHECK: 5.0

    state.transform = Doubler(4.0);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[3] = state.transform.apply(state.baseValue);
    // CHECK: 20.0
}
