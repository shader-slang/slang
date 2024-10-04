//TEST:SIMPLE(filecheck=ERROR): -target spirv -entry computeMain -stage compute

// Test senario where user calls into a user defined constructor
// of "private" visibility from an "internal" visible scope.

//ERROR: error 30600
public struct TestNested
{
    public int val1;
    public int val2;
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

    private __init(int in1)
    {
        this.val1 = in1;
        this.val2 = in1;
        this.nested1.val1 = in1;
        this.nested1.val2 = in1;
    }
};

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 2 };
	outputBuffer[0] = (true
        && val1.getVal1() == 2
        && val1.val2 == 2
        && val1.nested1.val1 == 2
        && val1.nested1.val2 == 2
        ) ? 1 : 0;
}