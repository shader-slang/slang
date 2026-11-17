// Regression test for #13509: an enclosing equality constraint on a
// generic parameter's associated type (`where C.Primitive == TrianglePrimitive`)
// must also apply to nested associated types reached through substitution
// (`C.Primitive.Attributes`), the same way it applies to a freshly spelled
// occurrence of the same type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

interface IPrimitive
{
    associatedtype Attributes;
}

struct TriangleAttributes
{
    float2 barycentricCoord;
}

struct TrianglePrimitive : IPrimitive
{
    typealias Attributes = TriangleAttributes;
    uint kind = 3;
}

interface IContext
{
    associatedtype Primitive : IPrimitive;
}

struct TriangleContext : IContext
{
    typealias Primitive = TrianglePrimitive;
}

// The field's type is checked in `Input`'s own generic context, so it is stored
// as an abstract lookup through `C.Primitive : IPrimitive` rather than reduced
// to a concrete type.
struct Input<C : IContext>
{
    C.Primitive.Attributes attributes;
}

struct PropertyInput<C : IContext>
{
    C.Primitive.Attributes stored;
    property C.Primitive.Attributes attributes { get { return stored; } }
}

struct PrimInput<C : IContext>
{
    C.Primitive prim;
}

// The core repro: the member access on the substituted field type must see the
// enclosing `C.Primitive == TrianglePrimitive` equality, and the value must
// also coerce to the concrete attribute type that equality implies.
uint readAttributes<C>(Input<C> input)
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    TriangleAttributes a = input.attributes;
    return uint(a.barycentricCoord.x * 100.0);
}

uint readViaLet<C>(Input<C> input)
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    let a = input.attributes;
    return uint(a.barycentricCoord.y * 100.0);
}

// Member access through a property whose type is the substituted abstract
// lookup.
uint readViaProperty<C>(PropertyInput<C> input)
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    return uint(input.attributes.barycentricCoord.x * 100.0);
}

uint readViaCall<C>(Input<C> input)
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    return uint(getAttributesUnconstrained<C>(input).barycentricCoord.x * 100.0);
}

uint readViaInterfaceConstraint<C : ITriCtx>(Input<C> input)
{
    return uint(input.attributes.barycentricCoord.x * 100.0);
}

// The fresh-spelling control: the same type written out in the constrained
// function's context resolves today, without the fix.
uint readFresh<C>()
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    TriangleAttributes a;
    a.barycentricCoord = float2(0.5, 0.5);
    C.Primitive.Attributes b = a;
    return uint(b.barycentricCoord.x * 100.0);
}

// The user-level workaround from the issue: spelling the deeper equality
// explicitly compiles on master, and must keep working alongside the new
// projection.
uint readWorkaround<C>(Input<C> input)
    where C : IContext
    where C.Primitive.Attributes == TriangleAttributes
{
    return uint(input.attributes.barycentricCoord.x * 100.0);
}

// The one-level control: the constraint applies to `C.Primitive` itself
// today, in both directions.
uint readOneLevel<C>(PrimInput<C> input)
    where C : IContext
    where C.Primitive == TrianglePrimitive
{
    return input.prim.kind;
}

// The constraint discovery must not depend on the order the `where` clauses
// are listed in: the equality constraint may precede the conformance bound.
uint readReversedOrder<C>(Input<C> input)
    where C.Primitive == TrianglePrimitive
    where C : IContext
{
    return uint(input.attributes.barycentricCoord.x * 100.0);
}

// A function declared *without* the equality constraint returns the nested
// type in its abstract form; a caller that has the constraint must still be
// able to use the result.
C.Primitive.Attributes getAttributesUnconstrained<C : IContext>(Input<C> input)
{
    return input.attributes;
}

// The interface-level spelling of the same constraint.
interface ITriCtx : IContext
{
    __constraint Primitive == TrianglePrimitive;
}

struct TriCtx : ITriCtx
{
    typealias Primitive = TrianglePrimitive;
}

// A three-level access `C.Prim2.Attributes.Data` must inherit the equality at
// each level of the chain.
interface IDataHolder
{
    associatedtype Data;
}

struct TriangleData
{
    uint payload;
}

struct TriangleAttributes2 : IDataHolder
{
    typealias Data = TriangleData;
    float2 barycentricCoord;
}

interface IPrimitive2
{
    associatedtype Attributes : IDataHolder;
}

struct TrianglePrimitive2 : IPrimitive2
{
    typealias Attributes = TriangleAttributes2;
}

interface IContext2
{
    associatedtype Prim2 : IPrimitive2;
}

struct Context2 : IContext2
{
    typealias Prim2 = TrianglePrimitive2;
}

struct Input3<C : IContext2>
{
    C.Prim2.Attributes.Data data;
}

uint read3<C>(Input3<C> input)
    where C : IContext2
    where C.Prim2 == TrianglePrimitive2
{
    TriangleData d = input.data;
    return d.payload + input.data.payload;
}

Input<TriangleContext> makeInput()
{
    Input<TriangleContext> input;
    input.attributes.barycentricCoord = float2(0.75, 0.25);
    return input;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Input<TriangleContext> input = makeInput();

    PropertyInput<TriangleContext> propertyInput;
    propertyInput.stored.barycentricCoord = float2(0.75, 0.25);

    Input<TriCtx> triCtxInput;
    triCtxInput.attributes.barycentricCoord = float2(0.75, 0.25);

    PrimInput<TriangleContext> primInput;
    primInput.prim.kind = 3;

    Input3<Context2> input3;
    input3.data.payload = 21;

    buf[0] = readAttributes(input);
    buf[1] = readViaLet(input);
    buf[2] = readViaProperty(propertyInput);
    buf[3] = readViaCall(input);
    buf[4] = readViaInterfaceConstraint(triCtxInput);
    buf[5] = readFresh<TriangleContext>();
    buf[6] = readWorkaround(input);
    buf[7] = readOneLevel(primInput);
    buf[8] = read3(input3);
    buf[9] = readReversedOrder(input);
}

// buf[5], buf[6] and buf[7] are the pre-fix controls (they compile on master
// without the congruence projection); the other seven cases fail to compile
// on master and need the fix.
//
// buf[0], buf[3], buf[4], buf[6], buf[9]: barycentricCoord.x * 100.
// buf[1]: barycentricCoord.y * 100.
// buf[2]: barycentricCoord.x * 100 through the property.
// buf[5]: the fresh-spelling control uses 0.5.
// buf[7]: the one-level control reads TrianglePrimitive::kind.
// buf[8]: 21 through the assignment plus 21 through direct member access.
//CHECK: 75
//CHECK-NEXT: 25
//CHECK-NEXT: 75
//CHECK-NEXT: 75
//CHECK-NEXT: 75
//CHECK-NEXT: 50
//CHECK-NEXT: 75
//CHECK-NEXT: 3
//CHECK-NEXT: 42
//CHECK-NEXT: 75
