//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    void test();
};

struct Foo<let GenericValue : uint> : IFoo
{
    void test()
    {
        printf("GenericValue=%d, value=%d\n", GenericValue, value);
    }

    uint value;
};

void testInterfaceAsParameter(IFoo foo)
{
    foo.test();
}

void testInterfaceAsGeneric<T:IFoo>(T foo)
{
    foo.test();
}

void main()
{
    // CHECK-COUNT-2:GenericValue=0, value=0
    Foo<0> foo0 = {0};
    testInterfaceAsParameter(foo0);
    testInterfaceAsGeneric(foo0);

    // CHECK-COUNT-2:GenericValue=1, value=1
    Foo<1> foo1 = {1};
    testInterfaceAsParameter(foo1);
    testInterfaceAsGeneric(foo1);

    // CHECK-COUNT-2:GenericValue=2, value=2
    Foo<2> foo2 = {2};
    testInterfaceAsParameter(foo2);
    testInterfaceAsGeneric(foo2);
}
