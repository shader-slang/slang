// Tests disabled: diagnostic 52010 not yet implemented (see #9906).
// Enable when __ref/__constref dynamic dispatch diagnostic is added.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cuda

// The diagnostic for __ref/__constref with interface types in dynamic
// dispatch is applied recursively: if a struct contains an array whose
// element type is (or transitively contains) an interface, the whole
// parameter is rejected because each array element would be packed
// into an AnyValue, making reference semantics invalid.

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

// Fixed-size array of interfaces: each element is an AnyValue.
struct ArrayContainer
{
    IFoo items[4];
}

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'ArrayContainer' cannot be used in dynamic dispatch context.
float processArrayRef(__ref ArrayContainer c)
{
    return c.items[0].getValue();
}

// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type 'ArrayContainer' cannot be used in dynamic dispatch context.
float processArrayConstref(__constref ArrayContainer c)
{
    return c.items[1].getValue();
}

// Multi-dimensional arrays are checked recursively through the element type.
struct MultiDimContainer
{
    IFoo grid[2][3];
}

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'MultiDimContainer' cannot be used in dynamic dispatch context.
float processMultiDim(__ref MultiDimContainer c)
{
    return c.grid[0][1].getValue();
}

// When a struct has multiple array fields, finding an interface in any
// of them is sufficient to reject the parameter.
struct MultipleArrays
{
    IFoo first[2];
    float dummy;
    IFoo second[3];
}

// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type 'MultipleArrays' cannot be used in dynamic dispatch context.
float processMultipleArrays(__constref MultipleArrays c)
{
    return c.first[0].getValue() + c.second[0].getValue();
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
    ArrayContainer ac;
    ac.items[0] = factory(dispatchThreadID.x);
    outputBuffer[0] = processArrayRef(ac);
    outputBuffer[1] = processArrayConstref(ac);

    MultiDimContainer mdc;
    mdc.grid[0][1] = factory(dispatchThreadID.x);
    outputBuffer[2] = processMultiDim(mdc);

    MultipleArrays ma;
    ma.first[0] = factory(dispatchThreadID.x);
    ma.second[0] = factory(dispatchThreadID.x);
    outputBuffer[3] = processMultipleArrays(ma);
}
