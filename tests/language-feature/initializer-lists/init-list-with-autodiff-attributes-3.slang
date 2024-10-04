//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct Test1 : IDifferentiable
{
    float3 a;
};
struct TestWrapper
{
    Test1 data1;
}

//CHECK: computeMain

[Differentiable]
TestWrapper getWrapper()
{
    return { float3(1) };
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    TestWrapper test1 = getWrapper();    
    outputBuffer[0] = true
        && test1.data1.a[0] == 1
        && test1.data1.a[1] == 1
        && test1.data1.a[2] == 1
        ;
}