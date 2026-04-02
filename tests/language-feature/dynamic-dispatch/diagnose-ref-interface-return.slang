// Tests that ref accessors returning interface types are rejected (#10244).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

// Returning a reference to an interface-typed value is incompatible
// with dynamic dispatch. Interface values are stored as existential
// tuples (RTTI + WitnessTableID + AnyValue), and a reference into
// that storage bypasses the pack/unpack mechanism needed for AnyValue
// marshalling. Read-only access may appear to work, but mutation
// through such a reference corrupts the AnyValue buffer and crashes
// at runtime (segfault observed on CPU target).
//
// This test verifies that the compiler rejects ref accessors when
// the value type is or contains a dynamic-dispatch interface.

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

// Scenario 1: Property ref accessor with direct interface type.
struct ContainerPropRef
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
        ref { return stored; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'IFoo' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 2: Subscript ref accessor with direct interface type.
struct CollectionSubRef
{
    IFoo items[2];
    __subscript(int i) -> IFoo
    {
        get { return items[i]; }
        ref { return items[i]; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'IFoo' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 3: Property ref accessor with struct containing interface.
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
//CHECK:^^^ 'ref' accessor returning type 'Wrapper' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 4: Property ref accessor with Tuple containing interface.
typealias TupleWithInterface = Tuple<IFoo, float>;

struct ContainerTupleRef
{
    TupleWithInterface t;
    property TupleWithInterface val
    {
        get { return t; }
        ref { return t; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Tuple<IFoo, float>' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 5: Property ref accessor with Optional containing interface.
struct ContainerOptionalRef
{
    Optional<IFoo> opt;
    property Optional<IFoo> val
    {
        get { return opt; }
        ref { return opt; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'Optional<IFoo>' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 6: Generic struct with interface type parameter.
// The check must apply generic substitutions to see through Box<IFoo>.
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
//CHECK:^^^ 'ref' accessor returning type 'Box<IFoo>' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 7: Array of interface type.
struct ContainerArrayRef
{
    IFoo arr[4];
    property IFoo[4] val
    {
        get { return arr; }
        ref { return arr; }
//CHECK:^^^ ref accessor incompatible with dynamic dispatch
//CHECK:^^^ 'ref' accessor returning type 'IFoo[4]' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// Scenario 8: Nested depth > 1 (struct containing struct containing interface).
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
//CHECK:^^^ 'ref' accessor returning type 'Outer' is incompatible with dynamic dispatch because interface types require AnyValue marshalling.
    }
}

// --- Positive tests: these should NOT produce error 52012 ---

// Positive 1: ref accessor on a concrete (non-interface) type is fine.
struct ContainerConcreteRef
{
    float stored;
    property float val
    {
        get { return stored; }
        ref { return stored; }
    }
}

// Positive 2: get-only accessor on interface type is fine (no ref).
struct ContainerGetOnly
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
    }
}

// Positive 3: set accessor (nonmutating) on interface type is fine.
struct ContainerSetOnly
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
        set { stored = newValue; }
    }
}

// Positive 4: COM interface with ref accessor is fine (COM uses vtable, not AnyValue).
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
