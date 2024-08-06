public struct TestNested
{
    private int val1 = 0;
    internal int val2;

    public int getVal1()
    {
        return val1;
    }
    public int getVal2()
    {
        return val2;
    }    
}

public struct Test
{
    private int val1 = 0;
    internal int val2;
    private TestNested nested1 = { 0 };
    private TestNested nested2 = { 0 };

    public static Test getTest()
    {
        return { 1, 2, { 3 }, { 4 }};
    }
    public int getVal1()
    {
        return val1;
    }
    public int getVal2()
    {
        return val2;
    }
    public TestNested getNested1()
    {
        return nested1;
    }
    public TestNested getNested2()
    {
        return nested2;
    }
};

public Test getTest()
{
    return { 1 };
}