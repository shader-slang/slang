//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

public struct TestNested
{
    private int val1 = 0;
    public int val2;
}

public struct Test
{
    private int val1 = 0;
    public int val2;
    public TestNested nested1 = { 0 };
    private TestNested nested2 = { 0 };

    static Test getTest()
    {
        return { 1, 2, { 3 }, { 4 } };
    }

    int getVal1()
    {
        return val1;
    }
    TestNested getNested2()
    {
        return nested2;
    }
};

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 1, { 2 } };
    Test val2 = Test::getTest();
	outputBuffer[0] = (true
        && val1.val2 == 1
        && val1.nested1.val2 == 2

        && val2.getVal1() == 1
        && val2.val2 == 2
        && val2.nested1.val2 == 3
        && val2.getNested2().val2 == 4         
        ) ? 1 : 0;

// BUF: 1
}