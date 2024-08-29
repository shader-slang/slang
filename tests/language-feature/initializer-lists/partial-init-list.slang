//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj

//CHECK: computeMain

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

public struct Test
{
    public uint a;
	public uint b;
};

Test getTest1()
{
    Test test = {1, 2};
    return test;
}
Test getTest2()
{
    Test test = {3};
    return test;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test1 = getTest1();
    Test test2 = getTest2();

//BUF: 1
    outputBuffer[0] = (true
        && test1.a == 1
        && test1.b == 2

        && test2.a == 3
        && test2.b == 0
        ) ? 1 : 0
        ;
}