//TEST:SIMPLE(filecheck=PASS): -target hlsl -entry computeMain -stage compute

struct Test
{
    uint a;
	uint b;
    __init(int b = 16)
    {
        this.b = b;
    }
};

RWStructuredBuffer<uint> outputBuffer;

//PASS: computeMain

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = Test();

   	outputBuffer[0] = test.a;
}