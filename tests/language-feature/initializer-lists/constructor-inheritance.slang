//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

// Test senario where we have constructors (partially defined through inheritance)
// and our init-list syntax calls into these constructors.

public struct TestDerived1
{
    private int val1 = 0;
    public int val2 = 0;
}
public struct Test1 : TestDerived1
{
    private int val3 = 0;
    public int val4 = 0;
}

public struct TestDerived2
{
    private int val1 = 0;
    public int val2 = 0;

    // This call overrides the memberwise constructor
    // of the base type.
    __init(int val)
    {
        this.val2 = val + 5;
    }
}
public struct Test2 : TestDerived2
{
    private int val3 = 0;
    public int val4 = 0;

    // Does not override memberwise constructor
    __init(int val)
    {
        val4 = val + 5; 
    }
}

public struct TestDerived3
{
    private int val1 = 0;
    public int val2 = 0;

    // Does not get called since the derived type
    // memberwise constructor is overloaded.
    __init(int val)
    {
        this.val2 = val + 5;
    }
}
public struct Test3 : TestDerived3
{
    private int val3 = 0;
    public int val4 = 0;
    // This call overrides memberwise constructor,
    // we ignore any inherited memberwise constructor.
    __init(int val1, int val2)
    {
        this.val2 = val1 + 5;
        this.val4 = val2 + 10;
    }
}

public struct TestDerived4
{
    private int val1 = 0;
    public int val2 = 0;
}
public struct Test4 : TestDerived4
{
    private int val3 = 0;
    public int val4 = 0;
    __init(int val4, int val2)
    {
        this.val2 = val2 + 10;
        this.val4 = val4;
    }
}

public struct TestDerived5
{
    private int val1 = 0;
    public int val2 = 0;
}
public struct Test5 : TestDerived5
{
    private int val3 = 0;
    public int val4 = 0;

    int getVal3()
    {
        return val3;
    }

    static Test5 callPrivateMemberwiseCtor()
    {
        return { 1, 2, 3 };
    }
}

public struct TestDerived6
{
    private int val1 = 0;
    public int val2 = 0;
}
public struct Test6 : TestDerived6
{
    private int val3 = 0;
    public int val4 = 0;

    __init(int val2, int val3, int val4)
    {
        this.val2 = val2*2;
        this.val3 = val3*2;
        this.val4 = val4*2;
    }

    int getVal3()
    {
        return val3;
    }

    static Test6 callPrivateMemberwiseCtor()
    {
        return { 1, 2, 3 };
    }
}

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test1 val1 = { 1, 2 };
    Test2 val2 = { 1, 2 };
    Test3 val3 = { 1, 2 };
    Test4 val4 = { 1, 2 };
    Test5 val5 = Test5::callPrivateMemberwiseCtor();
    Test6 val6 = Test6::callPrivateMemberwiseCtor();

	outputBuffer[0] = (true
        && val1.val2 == 1
        && val1.val4 == 2
        
        && val2.val2 == 6
        && val2.val4 == 2
        
        && val3.val2 == 6
        && val3.val4 == 12
        
        && val4.val2 == 12
        && val4.val4 == 1
        
        //&& val5.val2 == 1
        //&& val5.getVal3() == 2
        //&& val5.val4 == 3

        //&& val5.val2 == 2
        //&& val5.getVal3() == 4
        //&& val5.val4 == 6

        ) ? 1 : 0;

// BUF: 1
}