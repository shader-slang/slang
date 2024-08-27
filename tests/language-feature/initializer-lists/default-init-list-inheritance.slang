//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -Wno-41021

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

public struct TestDerived
{
//CHECK: = 5
    public uint a = 5;
//CHECK: = 0
	public uint b;
}

public struct Test : TestDerived
{
//CHECK: = 0
    public uint c;
//CHECK: = 6
	public uint d = 6;
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
        && test.c == 0
        && test.d == 6
        ) ? 1 : 0
        ;
}