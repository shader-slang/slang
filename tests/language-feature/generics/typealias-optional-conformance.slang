//TEST:INTERPRET(filecheck=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/10639
// Optional conformance should work through a typealias.

interface IFoo
{
    int getValue();
}

struct Bar<T> where optional T : IFoo
{
    int test()
    {
        if (T is IFoo)
            return 1;
        return 0;
    }
}

typealias Baz<T> = Bar<T>;

struct MyFoo : IFoo
{
    int getValue() { return 42; }
}

void main()
{
    // Direct usage: optional constraint is satisfied (MyFoo : IFoo).
    int a = Bar<MyFoo>().test();
    // Through typealias: must also satisfy the optional constraint.
    int b = Baz<MyFoo>().test();
    // Without conformance: constraint is not satisfied.
    int c = Bar<int>().test();
    int d = Baz<int>().test();
    printf("%d %d %d %d\n", a, b, c, d);
}

// CHECK: 1 1 0 0
