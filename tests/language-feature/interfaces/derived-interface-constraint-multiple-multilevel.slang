//TEST:INTERPRET(filecheck=CHECK):

// Two interface-level multi-level constraints that share a leading associated
// type: `__constraint A.B : ICnt; __constraint A.C : ICnt;`. Checking the first
// constraint drives the inheritance computation of `A`; that computation scans
// the interface's constraints and must not force-resolve the *second* (still
// unchecked) constraint, whose subject `A.C` would otherwise look up `C` on the
// in-progress `A` and spuriously report "member not found". Both `T.A.B` and
// `T.A.C` must gain `ICnt` as a base.

interface IVal
{
    int get();
}
interface ICnt
{
    int count();
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
    __constraint A.B : ICnt;
    __constraint A.C : ICnt;
}

struct VB : IVal, ICnt
{
    int n;
    int get() { return n; }
    int count() { return 100; }
}
struct VC : IVal, ICnt
{
    int n;
    int get() { return n; }
    int count() { return 200; }
}
struct BC : IBC
{
    typealias B = VB;
    typealias C = VC;
    VB getB() { VB v; v.n = 3; return v; }
    VC getC() { VC v; v.n = 7; return v; }
}
struct Root : IDeriv
{
    typealias A = BC;
    BC getA() { BC b; return b; }
}

// Uses the `A.B : ICnt` constraint to call `count()` on a `T.A.B` value.
int bCount<T : IDeriv>(T t) { return t.getA().getB().count() + t.getA().getB().get(); }
// Uses the `A.C : ICnt` constraint to call `count()` on a `T.A.C` value.
int cCount<T : IDeriv>(T t) { return t.getA().getC().count() + t.getA().getC().get(); }

void main()
{
    Root r;
    // bCount = 100 + 3 = 103; cCount = 200 + 7 = 207.
    // CHECK: 103 207
    printf("%d %d\n", bCount(r), cCount(r));
}
