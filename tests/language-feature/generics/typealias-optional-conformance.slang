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

// Two-level alias: transitive propagation of the optional constraint.
typealias Qux<T> = Baz<T>;

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
    // Two-level alias: transitive constraint propagation.
    int c = Qux<MyFoo>().test();
    // Without conformance: constraint is not satisfied.
    int d = Bar<int>().test();
    int e = Baz<int>().test();
    int f = Qux<int>().test();
    printf("%d %d %d %d %d %d\n", a, b, c, d, e, f);
}

// CHECK: 1 1 1 0 0 0

