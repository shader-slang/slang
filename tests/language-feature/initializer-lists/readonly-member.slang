//TEST:SIMPLE(filecheck=PASS): -target hlsl -allow-glsl -entry computeMain -stage compute

struct Test
{
    readonly uint a;
    writeonly uint b;
};

//PASS: computeMain
//PASS-NOT 

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = {2};

    outputBuffer[0] = true
        && test.b == 2;
        ;
}