//TEST:INTERPRET(filecheck=CHECK):

// A derived interface may re-declare (shadow) an inherited `property` requirement
// with a refined type. A two-level access in *value* position (`t.X.m()`) rooted
// at a type parameter `T : IDerived` must select the shadowing (derived) `X`, so
// that `.m()` -- reachable because the refined bound `IDerivedInner` refines
// `IInner` -- resolves. This complements the type-position `associatedtype` case
// in `assoctype-shadowed-nested-lookup.slang`.
//
// Regression test for shader-slang/slang#12513.

interface IInner
{
    int m();
}
interface IDerivedInner : IInner {}

interface IBase
{
    property X : IInner { get; }
}
interface IDerived : IBase
{
    property X : IDerivedInner { get; }
}

struct InnerImpl : IDerivedInner
{
    int n;
    int m() { return n; }
}

struct D : IDerived
{
    property X : InnerImpl
    {
        get { InnerImpl i; i.n = 7; return i; }
    }
}

int use<T : IDerived>(T t)
{
    return t.X.m();
}

void main()
{
    D d;
    // CHECK: 7
    printf("%d\n", use(d));
}
