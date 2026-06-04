//TEST:INTERPRET(filecheck=CHECK):

// Smoke / lowering guard related to shader-slang/slang#11465.
//
// #11465 is an internal `Type*`-identity invariant: the `This` of a substituted generic
// interface (reached via a `MemberDeclRef` base) must intern as the same `ThisType` as the
// `DirectDeclRef` path. That pointer-identity split is NOT observable from `.slang` surface —
// type comparison falls back to structural matching — so this test passes with AND without the
// fix; it does not fail-first. Its role is to guard that associated-type access and a
// `This`-returning requirement through a *substituted* generic interface (`IFoo<int>`) — the
// construction path #11465 concerns — still compiles, specializes and runs. The identity
// invariant itself is guarded by the interfaces/generics suite plus `getOrCreate`'s interning
// semantics (see the PR description).

interface ISummable
{
    int sum();
}

interface IFoo<T>
{
    associatedtype Params : ISummable;
    Params getParams();
    This combineWith(This other);
}

struct Pair : ISummable
{
    int a;
    int b;
    int sum() { return a + b; }
}

struct FooImpl<T> : IFoo<T>
{
    int tag;
    typealias Params = Pair;
    Params getParams()
    {
        Pair p;
        p.a = 3;
        p.b = 4;
        return p;
    }
    This combineWith(This other)
    {
        FooImpl<T> r;
        r.tag = this.tag + other.tag;
        return r;
    }
}

// `U : IFoo<int>` constrains on a substituted generic interface. Through `U` we reach both the
// `This`-returning requirement (`combineWith`) and the associated type (`getParams` →
// `U.Params`) — i.e. the `MemberDeclRef`-based `This` and associated-type access of #11465.
int run<U>(U a, U b) where U : IFoo<int>
{
    U combined = a.combineWith(b);
    let p = combined.getParams();
    return p.sum(); // Pair { 3, 4 }.sum() == 7
}

bool test()
{
    FooImpl<int> x;
    x.tag = 3;
    FooImpl<int> y;
    y.tag = 4;
    return run(x, y) == 7;
}

void main()
{
    // CHECK: pass
    if (test())
        printf("pass\n");
    else
        printf("fail\n");
}
