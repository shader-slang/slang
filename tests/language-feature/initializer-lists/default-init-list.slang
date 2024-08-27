//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

public struct Test
{
//CHECK: = 5
    public uint a = 5;
//CHECK: = 0
	public uint b;
};

Test getDefault()
{
    Test test = {};
    return test;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = getDefault();

//BUF: 1
    outputBuffer[0] = (true
        && test.a == 5
        && test.b == 0
        ) ? 1 : 0
        ;
}