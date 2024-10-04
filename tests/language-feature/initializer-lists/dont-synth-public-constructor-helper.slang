// Have a seperate file to ensure we are not trying to call an 'internal' synth'ed constructor.

public struct TestNested
{
    public int val1;
    public int val2;
}

public struct Test
{
#ifdef TEST_INTERNAL
    internal int val1;
#else
    private int val1;
#endif
    public int val2;
    public TestNested nested1;

    public int getVal1()
    {
        return val1;
    }
};