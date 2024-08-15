//TEST:SIMPLE(filecheck=ERROR): -target spirv -entry computeMain -stage compute

// Test senario where an 'private' member is not default-initialized
// for a struct but we call into a "internal" memberwise ctor.
// The called "internal" memberwise ctor should not be synth'ed.

//ERROR: error 30500
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
};

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 1, { 2, 3 } };
	outputBuffer[0] = (true
        && val1.getVal1() == 2
        && val1.val2 == 2
        && val1.nested1.val1 == 2
        && val1.nested1.val2 == 2
        ) ? 1 : 0;
}