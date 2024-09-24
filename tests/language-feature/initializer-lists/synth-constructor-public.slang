//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

public struct TestNested
{
    public int val1 = {};
    public int val2 = {};
}

public struct Test
{
    public int val1;
    public int val2;
    public TestNested nested1;
};

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val = { 1, 2, { 3, 4 } };
	outputBuffer[0] = (true
        && val.val1 == 1
        && val.val2 == 2
        && val.nested1.val1 == 3
        && val.nested1.val2 == 4
        ) ? 1 : 0;

// BUF: 1
}
