//TEST:INTERPRET(filecheck=CHECK):

// A derived interface may re-declare (shadow) an inherited `associatedtype`
// name with a refined bound:
//
//     interface IContext        { associatedtype Primitive : IPrimitive; }
//     interface ICustomContext : IContext { associatedtype Primitive : ICustomPrimitive; }
//
// A two-level nested lookup `C.Primitive.Attributes` rooted at a type parameter
// `C : ICustomContext` must resolve: `C.Primitive` selects the shadowing
// (derived) requirement, and because its bound `ICustomPrimitive` refines
// `IPrimitive`, the further access `.Attributes` (declared on `IPrimitive`) is
// reachable.
//
// Regression test for shader-slang/slang#12513.

interface IPrimitive
{
    associatedtype Attributes : IAttr;
}
interface ICustomPrimitive : IPrimitive {}

interface IAttr
{
    int value();
}

interface IContext
{
    associatedtype Primitive : IPrimitive;
}
interface ICustomContext : IContext
{
    associatedtype Primitive : ICustomPrimitive;
}

struct Result<T>
{
    T value;
}

interface IStage<C> where C : ICustomContext
{
    Result<C.Primitive.Attributes> invoke();
}

struct MyAttributes : IAttr
{
    int v;
    int value() { return v; }
}
struct MyPrimitive : ICustomPrimitive
{
    typealias Attributes = MyAttributes;
}
struct MyContext : ICustomContext
{
    typealias Primitive = MyPrimitive;
}

struct MyStage : IStage<MyContext>
{
    Result<MyAttributes> invoke()
    {
        Result<MyAttributes> r;
        r.value.v = 42;
        return r;
    }
}

// Drive the generic path so the nested associated-type lookup is exercised
// through the witness table, not just at the concrete conformance site.
int run<S : IStage<MyContext>>(S s)
{
    return s.invoke().value.value();
}

void main()
{
    MyStage s;
    // CHECK: 42
    printf("%d\n", run(s));
}
