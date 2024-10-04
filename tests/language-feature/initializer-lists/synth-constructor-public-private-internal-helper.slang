public struct NestedTest
{
    internal int val1 = 0;
    private int val2 = 0;
    public int val3;

    public int getVal1()
    {
        return val1;
    }
} 
public struct Test
{ 
    internal int val1 = 0;
    private int val2 = 0;
    public int val3;
    public NestedTest nested1 = { 0, 0 };

    public int getVal1()
    {
        return val1;
    }
}

public Test makeInternal()
{
    return { 1, 2, { 3, 4 } };
}