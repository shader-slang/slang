//TEST:INTERPRET(filecheck=CHECK):

int firstValue<let each D : int>() where nonempty(D)
{
    return __first(D);
}

int lastValue<let each D : int>() where nonempty(D)
{
    return __last(D);
}

int trimmedFirstCount<let each D : int>()
{
    return countof(__trimFirst(D));
}

int trimmedLastCount<let each D : int>()
{
    return countof(__trimLast(D));
}

int firstExpandedValue<let each D : int>() where nonempty(D)
{
    return __first(expand ((each D) + 1));
}

int passthrough(int x)
{
    return x;
}

enum MyEnum
{
    A = 1,
}

interface IDefaultPackValue
{
    __init();
    int getValue();
}

struct DefaultPackValue : IDefaultPackValue
{
    int value;

    __init()
    {
        value = 40;
    }

    int getValue()
    {
        return value;
    }
}

int firstExpandedValueWithRuntimeOffset<let each D : int>(int x) where nonempty(D)
{
    return __first(expand ((each D) + x));
}

int defaultArgWithMixedWitnesses<T, let each D : int>(int x = T().getValue() + __first(D))
    where nonempty(D)
    where T : IDefaultPackValue
{
    return x;
}

int consumeEnumPack<let each D : MyEnum>()
{
    return 123;
}

int singletonEnumTrimKeepsPackType<let each D : MyEnum>()
{
    return consumeEnumPack<__trimFirst(D)>();
}

void main()
{
    printf("%d\n", firstValue<10, 20, 30>());
    // CHECK: 10

    printf("%d\n", lastValue<10, 20, 30>());
    // CHECK: 30

    printf("%d\n", trimmedFirstCount<10, 20, 30>());
    // CHECK: 2

    printf("%d\n", trimmedLastCount<10, 20, 30>());
    // CHECK: 2

    printf("%d\n", trimmedFirstCount<>());
    // CHECK: 0

    printf("%d\n", trimmedLastCount<7>());
    // CHECK: 0

    printf("%d\n", firstExpandedValue<10, 20, 30>());
    // CHECK: 11

    int x = passthrough(5);
    printf("%d\n", firstExpandedValueWithRuntimeOffset<10, 20, 30>(x));
    // CHECK: 15

    printf("%d\n", defaultArgWithMixedWitnesses<DefaultPackValue, 2>());
    // CHECK: 42

    printf("%d\n", singletonEnumTrimKeepsPackType<MyEnum.A>());
    // CHECK: 123
}
