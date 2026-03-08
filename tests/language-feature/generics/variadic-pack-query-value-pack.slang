//TEST:INTERPRET(filecheck=CHECK):

int firstValue<let each D : int>() where nonempty(D)
{
    return __first(D);
}

int lastValue<let each D : int>() where nonempty(D)
{
    return __last(D);
}

int trimmedHeadCount<let each D : int>()
{
    return countof(__trimHead(D));
}

int trimmedTailCount<let each D : int>()
{
    return countof(__trimTail(D));
}

int firstExpandedValue<let each D : int>() where nonempty(D)
{
    return __first(expand ((each D) + 1));
}

int passthrough(int x)
{
    return x;
}

int firstExpandedValueWithRuntimeOffset<let each D : int>(int x) where nonempty(D)
{
    return __first(expand ((each D) + x));
}

void main()
{
    printf("%d\n", firstValue<10, 20, 30>());
    // CHECK: 10

    printf("%d\n", lastValue<10, 20, 30>());
    // CHECK: 30

    printf("%d\n", trimmedHeadCount<10, 20, 30>());
    // CHECK: 2

    printf("%d\n", trimmedTailCount<10, 20, 30>());
    // CHECK: 2

    printf("%d\n", trimmedHeadCount<>());
    // CHECK: 0

    printf("%d\n", trimmedTailCount<7>());
    // CHECK: 0

    printf("%d\n", firstExpandedValue<10, 20, 30>());
    // CHECK: 11

    int x = passthrough(5);
    printf("%d\n", firstExpandedValueWithRuntimeOffset<10, 20, 30>(x));
    // CHECK: 15
}
