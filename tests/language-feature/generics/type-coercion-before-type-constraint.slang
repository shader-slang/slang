//TEST:INTERPRET(filecheck=CHECK):

interface IHasValue
{
    int getValue();
}

struct ValueFromInt : IHasValue
{
    int value;

    __implicit_conversion(100)
    __init(int v)
    {
        value = v;
    }

    int getValue()
    {
        return value;
    }
}

// Keep the coercion constraint before the conformance constraint so the
// subtype witness substitution must skip over the coercion witness slot.
int readValue<T>(T x) where T(int) implicit where T : IHasValue
{
    return x.getValue();
}

void main()
{
    printf("%d\n", readValue<ValueFromInt>(1));
    // CHECK: 1
}
