//TEST:INTERPRET(filecheck=CHECK):

// Test for issue #7656: Compiler segfault with Ptr<T> extension referring to This
// This test ensures that the compiler doesn't segfault when 'This' is used in Ptr<T> extensions

public interface IThing
{
    int f(This other);
}

public extension Ptr<int>: IThing
{
    public int f(This other)
    {
        // Test that 'This' refers to Ptr<int>, not the pointed-to type int
        return sizeof(This) - sizeof(Ptr<int>) + 1;
    }
}

void main()
{
    Ptr<int> p;
    Ptr<int> other;
    int result = p.f(other);
    
    // CHECK: 1
    printf("%d\n", result);
}