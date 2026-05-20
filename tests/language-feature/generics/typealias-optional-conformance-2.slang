//TEST:INTERPRET(filecheck=CHECK):

// Additional coverage for https://github.com/shader-slang/slang/issues/10639
// Tests multiple independent optional constraints and non-generic typealias.

interface IA { int a(); }
interface IB { int b(); }

struct Dual<T> where optional T : IA where optional T : IB
{
    int test()
    {
        int r = 0;
        if (T is IA) r += 1;
        if (T is IB) r += 2;
        return r;
    }
}

typealias DualAlias<T> = Dual<T>;

struct AB : IA, IB { int a() { return 0; } int b() { return 0; } }
struct OnlyA : IA { int a() { return 0; } }

// Non-generic typealias of a constrained generic type (Bar<int>).
// The alias has no type parameter, so the optional constraint cannot be satisfied.
interface IFoo { int getValue(); }
struct Bar<T> where optional T : IFoo
{
    int test() { if (T is IFoo) return 1; return 0; }
}
typealias BarInt = Bar<int>;

void main()
{
    // Both constraints satisfied.
    int a = DualAlias<AB>().test();
    // Only IA satisfied.
    int b = DualAlias<OnlyA>().test();
    // Neither satisfied.
    int c = DualAlias<int>().test();
    // Non-generic alias: constraint not satisfied for int.
    int d = BarInt().test();
    printf("%d %d %d %d\n", a, b, c, d);
}

// CHECK: 3 1 0 0

