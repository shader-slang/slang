//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct InnerTest
{
    uint a;
};

struct Test
{
    InnerTest a;
    uint b;
    float2 c;
};

struct TestOuter : Test
{
    __init(float val)
    {
    }
};

//CHECK: warning 30505
//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    TestOuter test = {1, 1, float3(1).x, float3(1).z};

    outputBuffer[0] = true
        && test.a.a == 1
        && test.b == 1
        && test.c[0] == 1
        && test.c[1] == 1
        ;
}