//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct Test1 : IDifferentiable
{
    float3 a;
    no_diff float3 b;
};

//CHECK: computeMain

[Differentiable]
Test1 getWrapper()
{
    return { float3(1), float3(1) };
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test1 test1 = getWrapper();    
    outputBuffer[0] = true
        && test1.a[0] == 1
        && test1.a[1] == 1
        && test1.a[2] == 1
        && test1.b[0] == 1
        ;
}