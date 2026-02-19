// Tests disabled: diagnostic 52010 not yet implemented (see #9906).
// Enable when __ref/__constref dynamic dispatch diagnostic is added.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl

// Test that __ref and __constref parameters with struct types
// containing interface members produce diagnostic errors in
// dynamic dispatch contexts.

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
}

struct Container
{
    IFoo member;
}

// Scenario 1: Struct containing interface passed by __ref
// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'Container' cannot be used in dynamic dispatch context.
float processContainerRef(__ref Container c)
{
    return c.member.getValue();
}

// Scenario 2: Struct containing interface passed by __constref
// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type 'Container' cannot be used in dynamic dispatch context.
float processContainerConstref(__constref Container c)
{
    return c.member.getValue();
}

// Scenario 3: Nested struct containing interface passed by __ref
struct Inner
{
    IFoo iface;
}

struct Outer
{
    Inner inner;
}

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'Outer' cannot be used in dynamic dispatch context.
float processNested(__ref Outer o)
{
    return o.inner.iface.getValue();
}

IFoo factory(uint id)
{
    if (id == 0) return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Container c;
    c.member = factory(dispatchThreadID.x);
    outputBuffer[0] = processContainerRef(c);
    outputBuffer[1] = processContainerConstref(c);

    Outer o;
    o.inner.iface = factory(dispatchThreadID.x);
    outputBuffer[2] = processNested(o);
}
