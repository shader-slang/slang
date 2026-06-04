//TEST:INTERPRET(filecheck=CHECK):

// Regression for shader-slang/slang#11465.
//
// The `This` type of a *substituted* generic interface (e.g. `IFoo<int>`) is reached
// through a `MemberDeclRef` base, whereas the `This` reached through a `DirectDeclRef`
// base is built as a `ThisType`. Before the fix these interned to two distinct `Type*`
// for the same `ThisTypeDecl`, breaking pointer-identity type comparison. This test
// exercises a substituted generic interface that has both an associated type and a
// `This`-returning requirement, used through a generic constraint `where U : IFoo<int>`,
// so the `MemberDeclRef`-based `This` is constructed, specialized and lowered.

interface IFoo<T>
{
    associatedtype Params;
    Params getParams();
    int score();
    This combineWith(This other);
}

struct Pair
{
    int a;
    int b;
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
    int score() { return this.tag; }
    This combineWith(This other)
    {
        FooImpl<T> r;
        r.tag = this.score() + other.score();
        return r;
    }
}

int run<U>(U a, U b) where U : IFoo<int>
{
    U combined = a.combineWith(b);
    return combined.score();
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
