// Tests disabled: diagnostic 52010 covers __ref/__constref parameters (#9906)
// but has not been extended to ref accessor returns yet (see #10244).
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl

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
// The ref accessor implicitly returns a pointer to the existential
// tuple, which is unsound for dynamic dispatch.
struct ContainerPropRef
{
    IFoo stored;
    property IFoo val
    {
        get { return stored; }
        // CHECK: ([[# @LINE+1]]): error 52010
        ref { return stored; }
    }
}

// Scenario 2: Subscript ref accessor with direct interface type.
struct CollectionSubRef
{
    IFoo items[2];
    __subscript(int i) -> IFoo
    {
        get { return items[i]; }
        // CHECK: ([[# @LINE+1]]): error 52010
        ref { return items[i]; }
    }
}

// Scenario 3: Property ref accessor with struct containing interface.
// The struct has an interface-typed field, so a reference to it also
// provides indirect access to AnyValue-packed storage.
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
        // CHECK: ([[# @LINE+1]]): error 52010
        ref { return w; }
    }
}

// Scenario 4: Property ref accessor with Tuple containing interface.
// The check should recurse into generic compound types, just as it
// does for __ref/__constref parameters.
typealias TupleWithInterface = Tuple<IFoo, float>;

struct ContainerTupleRef
{
    TupleWithInterface t;
    property TupleWithInterface val
    {
        get { return t; }
        // CHECK: ([[# @LINE+1]]): error 52010
        ref { return t; }
    }
}

// Scenario 5: Property ref accessor with Optional containing interface.
struct ContainerOptionalRef
{
    Optional<IFoo> opt;
    property Optional<IFoo> val
    {
        get { return opt; }
        // CHECK: ([[# @LINE+1]]): error 52010
        ref { return opt; }
    }
}

// CHECK-NOT: error 52010

IFoo factory(uint id)
{
    if (id == 0) return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

// The diagnostic fires at the ref accessor declaration for all five
// scenarios above. Only scenarios 1-3 are exercised below to show
// typical consumer patterns; scenarios 4-5 (Tuple, Optional) don't
// need runtime usage since the error is caught at declaration time.
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
