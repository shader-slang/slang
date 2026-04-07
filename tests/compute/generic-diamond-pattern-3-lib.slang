//TEST_IGNORE_FILE:

// Module 1: Diamond conformance interfaces, BottomImpl, and GenFoo.

public interface Top
{
};

public interface MidLeft : Top
{
};

public interface MidRight : Top
{
};

public interface Bottom : MidLeft, MidRight
{
};

public struct BottomImpl : Bottom
{
    public float x;
};

public struct GenFoo<T : Top>
{
    public T x;
};

public GenFoo<BottomImpl> getGenFoo(uint id)
{
    GenFoo<BottomImpl> result;
    result.x.x = float(id) * 3.0;
    return result;
}
