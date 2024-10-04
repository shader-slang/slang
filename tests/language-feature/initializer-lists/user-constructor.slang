//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

// Test senario where user defines constructors
// and our init-list syntax calls into those constructors

public struct TestNested
{
    public int val1 = {};
    public int val2 = {};
}

public struct Test
{
    private int val1;
    public int val2;
    public TestNested nested1;

    int getVal1()
    {
        return val1;
    }

    __init(int in1, TestNested in2)
    {
        this.val1 = in1;
        this.val2 = in1;
        this.nested1 = in2;
        this.nested1.val1 = in1;
    }

    __init(int in1)
    {
        this.val1 = in1;
        this.val2 = in1;
        this.nested1.val1 = in1;
        this.nested1.val2 = in1;
    }

    __init()
    {
        this.val1 = 5;
        this.val2 = 5;
        this.nested1.val1 = 5;
        this.nested1.val2 = 5;
    }
};

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 1, { 2, 3 } };
    Test val2 = { 2 };
    Test val3 = {};
	outputBuffer[0] = (true
        && val1.getVal1() == 1
        && val1.val2 == 1
        && val1.nested1.val1 == 1
        && val1.nested1.val2 == 3

        && val2.getVal1() == 2
        && val2.val2 == 2
        && val2.nested1.val1 == 2
        && val2.nested1.val2 == 2

        && val3.getVal1() == 5
        && val3.val2 == 5
        && val3.nested1.val1 == 5
        && val3.nested1.val2 == 5
        ) ? 1 : 0;

// BUF: 1
}
