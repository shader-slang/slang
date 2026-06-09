//TEST:INTERPRET(filecheck=CHECK):

// Additional coverage for https://github.com/shader-slang/slang/issues/10639
// Tests multiple independent optional constraints and non-generic typealias,
// plus edge cases in visitTypeDefDecl (value params, mixed constraints,
// partial application, equality constraints, non-DeclRefType bodies).

interface IA { int a(); }
interface IB { int b(); }
interface IFoo { int getValue(); }

struct AB : IA, IB    { int a() { return 0; } int b() { return 0; } }
struct OnlyA : IA     { int a() { return 0; } }
struct MyFoo : IFoo   { int getValue() { return 42; } }
struct JustB : IB     { int b() { return 7; } }

// --- Original tests ---

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

// Non-generic typealias of a constrained generic type (Bar<int>).
// The alias has no type parameter, so the optional constraint cannot be satisfied.
struct Bar<T> where optional T : IFoo
{
    int test() { if (T is IFoo) return 1; return 0; }
}
typealias BarInt = Bar<int>;

// --- Case 1: equality constraint ---
// isEqualityConstraint must be copied to the synthesized constraint decl
// (synConstraintDecl->isEqualityConstraint = constraintDecl->isEqualityConstraint).
// Verify the alias compiles and can be instantiated for both T==int and T!=int.
struct EqBody<T> where optional T == int {}
typealias EqAlias<T> = EqBody<T>;

// --- Case 2: value param in the alias ---
// ordinaryParamCount must count the let N : int param (GenericValueParamDecl)
// so that argIdx starts past both ordinary params and lands on the constraint slot.
struct WithValue<let N : int, T> where optional T : IFoo
{
    int test() { if (T is IFoo) return N; return 0; }
}
typealias WithValueAlias<let N : int, T> = WithValue<N, T>;

// --- Case 3: mixed required + optional constraints ---
// T : IA is explicitly forwarded on the alias (required constraints are not
// auto-propagated); only the optional U : IB slot undergoes NoneWitness substitution.
// Verifies that argIdx advances correctly past the non-NoneWitness required slot.
struct Mixed<T, U> where T : IA where optional U : IB
{
    int test() { if (U is IB) return 1; return 0; }
}
typealias MixedAlias<T : IA, U> = Mixed<T, U>;

// --- Case 4: partial-application alias ---
// U is bound to int; int does not satisfy IFoo, so the NoneWitness for that
// constraint slot remains. The constraint must not be spuriously transferred to T:
// Half<MyFoo>().test() must return 0 (because U==int, not T).
struct Pair2<T, U> where optional U : IFoo
{
    int test() { if (U is IFoo) return 1; return 0; }
}
typealias Half<T> = Pair2<T, int>;

// --- Case 5: non-DeclRefType body ---
// functype(T)->int is not a DeclRefType, so visitTypeDefDecl hits the early
// return and must not attempt any constraint propagation.
typealias FnAlias<T> = functype(T) -> int;
int callFn(FnAlias<int> f, int x) { return f(x); }

void main()
{
    // Original: both constraints satisfied.
    int a = DualAlias<AB>().test();
    // Original: only IA satisfied.
    int b = DualAlias<OnlyA>().test();
    // Original: neither satisfied.
    int c = DualAlias<int>().test();
    // Original: non-generic alias, constraint not satisfied for int.
    int d = BarInt().test();

    // Case 1: equality constraint alias compiles for matching and non-matching T.
    EqAlias<int> _eq1;
    EqAlias<float> _eq2;

    // Case 2: value param alias.
    int e = WithValueAlias<5, MyFoo>().test();  // MyFoo satisfies IFoo: 5
    int f = WithValueAlias<5, int>().test();    // int doesn't satisfy IFoo: 0

    // Case 3: mixed required + optional.
    int g = MixedAlias<OnlyA, JustB>().test();  // U=JustB satisfies IB: 1
    int h = MixedAlias<OnlyA, int>().test();    // U=int doesn't satisfy IB: 0

    // Case 4: partial application — U=int, never satisfies IFoo regardless of T.
    int i = Half<float>().test();  // 0
    int j = Half<MyFoo>().test();  // also 0 — T doesn't affect U's constraint

    // Case 5: functype alias is callable.
    int k = callFn((int x) => x + 1, 5);  // 6

    printf("%d %d %d %d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h, i, j, k);
}

// CHECK: 3 1 0 0 5 0 1 0 0 0 6
