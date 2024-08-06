//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

public struct TestNested
{
    private int val1;
    private int val2;

    int getVal1()
    {
        return val1;
    }
    int getVal2()
    {
        return val2;
    }    
}

public struct Test
{
    private int val1;
    private int val2;
    private TestNested nested1;

    static Test getTest()
    {
        return { 1, 2, {}};
    }

    int getVal1()
    {
        return val1;
    }
    int getVal2()
    {
        return val2;
    }
    TestNested getNested1()
    {
        return nested1;
    }
};

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = {};
    Test val2 = Test::getTest();
    outputBuffer[0] = (true
        && val1.getVal1() == 0
        && val1.getNested1().getVal1() == 0

        && val2.getVal1() == 1
        && val2.getVal2() == 2
        && val2.getNested1().getVal1() == 0
        && val2.getNested1().getVal2() == 0
        ) ? 1 : 0;

// BUF: 1
}