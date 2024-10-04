//TEST:SIMPLE(filecheck=CHECK): -target hlsl -allow-glsl -entry computeMain -stage compute

struct Test
{
    readonly uint a;
    writeonly uint b;
};

//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = {2};

    outputBuffer[0] = true
        && test.b == 2;
        ;
}