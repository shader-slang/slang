//TEST:INTERPRET(filecheck=CHECK):

// An interface-level `__constraint` whose left-hand side is a *multi-level*
// associated-type lookup (`A.B.C`), as opposed to the single-level forms
// (`DataType`, `DataType == This`) the other tests cover. This exercises the
// depth>1 access-chain decompose/match path in `calcInheritanceInfo`: the
// constraint subject `This.A.B.C` must be matched against the access
// `T.A.B.C` (a chain of three associated-type lookups) so that `Bar` is
// surfaced as a base of `T.A.B.C` in generic code constrained by `IDerived`.

struct Bar
{
    int v;
    int barMethod() { return v + 100; }
}

interface ILevel2
{
    associatedtype C;
    C getC();
}
interface ILevel1
{
    associatedtype B : ILevel2;
    B getB();
}
interface IFoo
{
    associatedtype A : ILevel1;
    A getA();
}

interface IDerived : IFoo
{
    // For any `T : IDerived`, the three-level access `T.A.B.C` is exactly `Bar`.
    __constraint A.B.C == Bar;
}

struct L2 : ILevel2
{
    typealias C = Bar;
    Bar c;
    Bar getC() { return c; }
}
struct L1 : ILevel1
{
    typealias B = L2;
    L2 b;
    L2 getB() { return b; }
}
struct Root : IDerived
{
    typealias A = L1;
    L1 a;
    L1 getA() { return a; }
}

int useDerived<T : IDerived>(T t)
{
    // `t.getA().getB().getC()` has type `T.A.B.C`. This type-checks as a call to
    // `Bar.barMethod()` only because `__constraint A.B.C == Bar` makes the
    // compiler treat `T.A.B.C` as `Bar` (the multi-level-lookup constraint must
    // be discovered when computing the inheritance of `T.A.B.C`).
    return t.getA().getB().getC().barMethod();
}

void main()
{
    Root r;
    r.a.b.c.v = 5;

    // 5 + 100 = 105
    // CHECK: 105
    printf("%d\n", useDerived(r));
}
