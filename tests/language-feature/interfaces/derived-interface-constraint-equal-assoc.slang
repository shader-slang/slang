//TEST:INTERPRET(filecheck=CHECK):

// An interface-level `__constraint A == B` between *two equal-depth associated
// types*. The equality makes `T.A` and `T.B` each a base of the other, so
// computing the inheritance of one re-enters the computation of the other -- a
// *benevolent* cycle. `calcInheritanceInfo` breaks it by skipping the
// in-progress ancestor (and not caching the resulting partial, contextual
// list), so the equality is usable in BOTH directions. Before that handling
// this produced a "cyclic reference in inheritance graph" error.
//
// (Distinct from `derived-interface-constraint*.slang`, where one endpoint is
// `This` or the two endpoints have different depths -- neither of which forms
// an equal-depth associated-type cycle.)

interface IVal
{
    int get();
}

interface IPair
{
    associatedtype A : IVal;
    associatedtype B : IVal;
    A getA();
    B getB();
}

interface IEqPair : IPair
{
    __constraint A == B;
}

struct V : IVal
{
    int n;
    int get() { return n; }
}

struct P : IEqPair
{
    typealias A = V;
    typealias B = V;
    V getA() { V v; v.n = 3; return v; }
    V getB() { V v; v.n = 4; return v; }
}

// `t.getA()` has type `T.A`; returning it where `T.B` is expected relies on the
// `A == B` equality (and vice versa for `asA`). Both directions must resolve.
T.B asB<T : IEqPair>(T t) { return t.getA(); }
T.A asA<T : IEqPair>(T t) { return t.getB(); }

void main()
{
    P p;
    // asB(p) = getA() = 3 (used as B); asA(p) = getB() = 4 (used as A).
    // CHECK: 3 4
    printf("%d %d\n", asB(p).get(), asA(p).get());
}
