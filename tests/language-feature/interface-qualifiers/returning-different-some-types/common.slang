public interface IFoo
{
    public int getVal();
    [mutating]
    public void setVal(int v);
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
    [mutating]
    public void setVal(int v)
    {
        a = v;
    }
};