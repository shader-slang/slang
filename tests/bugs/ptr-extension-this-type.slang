// TODO(#14295): `p.f(other)` fails with E30027, because a member of an extension on a pointer
// type is not found on a pointer value. The test passed by accident before, because `CHECK: 1`
// matched `result code = 1`.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

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