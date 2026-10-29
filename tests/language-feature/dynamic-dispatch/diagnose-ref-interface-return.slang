// Tests ref accessors whose return types contain existential boxes (#10244).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

// These cases use the legacy convention of writing `IFoo` where the stored value has type
// `dyn IFoo`. For boxes over non-COM, non-builtin interfaces, the compiler stores concrete
// payloads in AnyValue storage. Taking a reference into that storage bypasses the packing
// and unpacking used to access those payloads, so mutation through the reference can corrupt
// the storage.
//
// The compiler rejects `ref` accessors whose return types contain such boxes, including
// boxes nested inside other types.

[anyValueSize(16)]
interface IFoo
{
    float getValue();
    [mutating] void setValue(float v);
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
    [mutating] void setValue(float nv) { v = nv; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
    [mutating] void setValue(float nv) { v = nv; }
}

// Scenario 1: Property ref accessor returning `dyn IFoo`.
struct ContainerPropRef
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
        ref { return stored; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'dyn IFoo' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 2: Subscript ref accessor returning `dyn IFoo`.
struct CollectionSubRef
{
    IFoo items[2];
    __subscript(int i) -> IFoo
    {
        get { return items[i]; }
        ref { return items[i]; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'dyn IFoo' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 3: Property ref accessor returning a struct that contains an existential box.
struct Wrapper
{
    IFoo item;
}

struct ContainerStructRef
{
    Wrapper w;
    property Wrapper val
    {
        get { return w; }
        ref { return w; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Wrapper' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 4: Property ref accessor returning a tuple that contains an existential box.
typealias TupleWithInterface = Tuple<IFoo, float>;

struct ContainerTupleRef
{
    TupleWithInterface t;
    property TupleWithInterface val
    {
        get { return t; }
        ref { return t; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Tuple<dyn IFoo, float>' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 5: Property ref accessor returning an optional existential box.
struct ContainerOptionalRef
{
    Optional<IFoo> opt;
    property Optional<IFoo> val
    {
        get { return opt; }
        ref { return opt; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Optional<dyn IFoo>' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 6: A ref accessor returning `Box<dyn IFoo>`.
// In the legacy spelling `Box<IFoo>`, the type argument `IFoo` is implicitly converted to
// `dyn IFoo`, so the `T item` field has type `dyn IFoo`.
struct Box<T>
{
    T item;
}

struct ContainerGenericRef
{
    Box<IFoo> b;
    property Box<IFoo> val
    {
        get { return b; }
        ref { return b; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Box<dyn IFoo>' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 7: Array of existential boxes.
struct ContainerArrayRef
{
    IFoo arr[4];
    property IFoo[4] val
    {
        get { return arr; }
        ref { return arr; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'dyn IFoo[4]' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// Scenario 8: A struct whose nested struct contains an existential box.
struct Inner
{
    IFoo item;
}

struct Outer
{
    Inner inner;
}

struct ContainerNestedRef
{
    Outer o;
    property Outer val
    {
        get { return o; }
        ref { return o; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Outer' is incompatible with dynamic dispatch because the compiler uses AnyValue marshalling for an existential box in the return type.
    }
}

// --- Positive tests: these should NOT produce error 52012 ---

// Positive 1: A ref accessor returning `float` is allowed.
struct ContainerConcreteRef
{
    float stored;
    property float val
    {
        get { return stored; }
        ref { return stored; }
    }
}

// Positive 2: A get-only accessor returning an existential box is allowed.
struct ContainerGetOnly
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
    }
}

// Positive 3: Get/set accessors for an existential box are allowed.
struct ContainerSetOnly
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
        set { stored = newValue; }
    }
}

// Positive 4: A ref accessor returning a COM box is allowed because the compiler uses a vtable
// for COM interface dispatch instead of AnyValue marshalling.
[COM("BE18F5D2-4522-4AB0-A6EE-1D157FA2B084")]
interface ICom
{
    float getValue();
}

struct ContainerComRef
{
    ICom stored;
    property ICom val
    {
        get { return stored; }
        ref { return stored; }
    }
}

// CHECK-NOT: error 52012

IFoo factory(uint id)
{
    if (id == 0) return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ContainerPropRef c1;
    c1.stored = factory(dispatchThreadID.x);
    outputBuffer[0] = c1.val.getValue();

    CollectionSubRef c2;
    c2.items[0] = factory(0);
    c2.items[1] = factory(1);
    outputBuffer[1] = c2[0].getValue();

    ContainerStructRef c3;
    c3.w.item = factory(dispatchThreadID.x);
    outputBuffer[2] = c3.val.item.getValue();
}
