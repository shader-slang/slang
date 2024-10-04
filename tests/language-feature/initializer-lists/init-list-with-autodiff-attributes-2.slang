//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct Test : IDifferentiable
{
    float3 a;
    no_diff float3 b;
};

//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test.Differential test1 = {};
    Test test2 = {};
    outputBuffer[0] = true
        && test1.a[0] == 0
        && test1.a[1] == 0
        && test1.a[2] == 0

        && test1.a[0] == 0
        && test1.a[1] == 0
        && test1.a[2] == 0
        && test1.b[0] == 0
        && test1.b[1] == 0
        && test1.b[2] == 0
        ;
}