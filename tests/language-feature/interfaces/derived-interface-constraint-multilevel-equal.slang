//TEST:INTERPRET(filecheck=CHECK):

// An interface-level equality between two *multi-level, equal-depth* associated-
// type accesses: `__constraint A.B == A.C`. Like `__constraint A == B` this is a
// benevolent cycle (`T.A.B` and `T.A.C` each a base of the other), but the
// accesses are depth-2 -- exercising the chain-matching path with a shared
// prefix `A`. Must be usable in both directions.

interface IVal
{
    int get();
}
interface IBC
{
    associatedtype B : IVal;
    associatedtype C : IVal;
    B getB();
    C getC();
}
interface IFoo
{
    associatedtype A : IBC;
    A getA();
}
interface IDeriv : IFoo
{
    __constraint A.B == A.C;
}

struct V : IVal
{
    int n;
    int get() { return n; }
}
struct BC : IBC
{
    typealias B = V;
    typealias C = V;
    V getB() { V v; v.n = 3; return v; }
    V getC() { V v; v.n = 7; return v; }
}
struct Root : IDeriv
{
    typealias A = BC;
    BC getA() { BC b; return b; }
}

// `t.getA().getB()` is `T.A.B`; returning it where `T.A.C` is expected relies on
// `A.B == A.C` (and vice versa).
T.A.C asC<T : IDeriv>(T t) { return t.getA().getB(); }
T.A.B asB<T : IDeriv>(T t) { return t.getA().getC(); }

void main()
{
    Root r;
    // asC = getB() = 3 (used as A.C); asB = getC() = 7 (used as A.B).
    // CHECK: 3 7
    printf("%d %d\n", asC(r).get(), asB(r).get());
}
