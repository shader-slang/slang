public dyn interface IFoo
{
    public int getVal();
}

public struct StructForIFoo1 : IFoo
{
    public __init(int v)
    {
        a = v;
    }
    int a = 0;
    public int getVal()
    {
        return a;
    }
};
public struct StructForIFoo2 : IFoo
{
    public __init(int v)
    {
        a = v*10;
    }
    int a = 0;
    public int getVal()
    {
        return a;
    }
};