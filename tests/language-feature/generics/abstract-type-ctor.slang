//TEST:INTERPRET(filecheck=CHECK):

// Verify constructor lookup still works through abstract or symbolic types
// that arise from recursive pack-branch structures.

interface IFoo
{
    __init(int x);
    __init(float x, float y);
    int eval();
}

struct FooImpl : IFoo
{
    int value;

    __init(int x)
    {
        value = x + 1;
    }

    __init(float x, float y)
    {
        value = int(x * 10.0f + y * 10.0f);
    }

    int eval()
    {
        return value;
    }
}

int fromIntCtor<T : IFoo>()
{
    T value = {41};
    return value.eval();
}

int fromFloatCtor<T : IFoo>()
{
    T value = {1.0f, 2.0f};
    return value.eval();
}

void main()
{
    printf("%d\n", fromIntCtor<FooImpl>());
    // CHECK: 42

    printf("%d\n", fromFloatCtor<FooImpl>());
    // CHECK: 30
}
