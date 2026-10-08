//TEST:INTERPRET(filecheck=CHECK):

// Two generic `where` constraints that share a leading associated-type access:
// `where T.A.B : ICnt where T.A.C : ICnt`. Checking the first drives the
// inheritance computation of `T.A`; the constraint scan must not force-resolve
// the second (still-unchecked) constraint, whose subject `T.A.C` would otherwise
// look up `C` on the in-progress `T.A` and spuriously report "member not found".
// This is the generic-parameter analogue of the interface-level case in
// tests/language-feature/interfaces/derived-interface-constraint-multiple-multilevel.slang;
// both scans share `tryResolveConstraintTypes`.

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
struct FooS : IFoo
{
    typealias A = BC;
    BC getA() { BC b; return b; }
}

// Uses both `T.A.B : ICnt` and `T.A.C : ICnt` to call `count()`.
int sum<T : IFoo>(T t) where T.A.B : ICnt where T.A.C : ICnt
{
    return t.getA().getB().count() + t.getA().getC().count();
}

void main()
{
    FooS f;
    // 100 + 200 = 300.
    // CHECK: 300
    printf("%d\n", sum(f));
}
