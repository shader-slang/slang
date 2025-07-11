//TEST:COMPILE: -target hlsl

// Test for issue #7656: Compiler segfault with Ptr<T> extension referring to This
// This test ensures that the compiler doesn't segfault when 'This' is used in Ptr<T> extensions

public interface IThing
{
    void f(This other);
}

public extension Ptr<int>: IThing
{
    public void f(This other)
    {
        // This function should compile without segfault
        // 'This' should refer to Ptr<int>, not the pointed-to type int
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    // The main test is that the extension compiles without segfault
    // The 'This' type resolution should work correctly
}