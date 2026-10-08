//TEST:INTERPRET(filecheck=CHECK):

// An interface-level equality between multi-level accesses of *different depth*:
// `__constraint A.B == A.B.C` (depth 2 == depth 3). This is the reduce-to-
// shallower direction: `T.A.B.C` gains `T.A.B` as a base (and the deeper
// direction is cut by the `selfChainDepth` guard so it terminates), so a
// `T.A.B.C` value is usable where `T.A.B` is expected.

interface IVal
{
    int get();
}
// A node whose associated `C` is itself (so `A.B.C == A.B` is satisfiable).
interface IRecB : IVal
{
    associatedtype C : IRecB;
    C getC();
}
interface IHasB
{
    associatedtype B : IRecB;
    B getB();
}
interface IFoo
{
    associatedtype A : IHasB;
    A getA();
}
interface IDeriv : IFoo
{
    __constraint A.B == A.B.C;
}

struct Node : IRecB
{
    int n;
    typealias C = Node;
    int get() { return n; }
    Node getC() { Node m; m.n = n + 1; return m; }
}
struct HB : IHasB
{
    typealias B = Node;
    Node getB() { Node m; m.n = 5; return m; }
}
struct Root : IDeriv
{
    typealias A = HB;
    HB getA() { HB h; return h; }
}

// `t.getA().getB().getC()` is `T.A.B.C`; returning it where `T.A.B` is expected
// relies on `A.B == A.B.C`.
T.A.B reduce<T : IDeriv>(T t) { return t.getA().getB().getC(); }

void main()
{
    Root r;
    // getB() = Node{5}, .getC() = Node{6}, used as A.B.
    // CHECK: 6
    printf("%d\n", reduce(r).get());
}
