// NOTE: 'warning of uninitialized values system' does not handle assignments to `this`.
// For example, `this = {};`.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -Wno-41020
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj -xslang -Wno-41020

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

public struct Test
{
//CHECK: = 5
    public uint a;
//CHECK: = 0
	public uint b;
    __init()
    {
        // This code will do 2 things:
        // 1. call `$ZeroInit()`
        // 2. assign `a = 5`
        this = {};
        a = 5;
    }
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