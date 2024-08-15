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


//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = Test();

    // BUF: 16
   	outputBuffer[0] = test.a;
}