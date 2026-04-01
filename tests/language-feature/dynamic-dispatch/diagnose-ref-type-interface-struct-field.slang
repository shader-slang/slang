// Using Ref<T> as a struct field type is not supported. Reference types
// (Ref, RefParam, BorrowInParam) are designed for function parameters
// and return values; they cannot be stored in struct fields because
// there is no stable storage address to reference across struct copies.
//
// When T is an interface type, this is doubly problematic: the existential
// value is stored as an AnyValue-packed tuple, and a reference into that
// buffer would bypass the pack/unpack marshalling needed for dynamic
// dispatch.
//
// Currently this triggers an internal compiler error (assert failure in
// LoweredValInfo) instead of a user-friendly diagnostic. These tests are
// disabled until the compiler produces a proper error message.

//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain

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

// Ref<IFoo> as a struct field: should be diagnosed rather than ICE.
struct RefInterfaceField
{
    // CHECK: ([[# @LINE+1]]): error
    Ref<IFoo> field;
}

// Ref<ConcreteType> as a struct field: same underlying issue.
struct RefConcreteField
{
    // CHECK: ([[# @LINE+1]]): error
    Ref<float> field;
}

// Nested: struct containing another struct with Ref<IFoo> field.
struct InnerRef
{
    // CHECK: ([[# @LINE+1]]): error
    Ref<IFoo> iface;
}

struct OuterRef
{
    InnerRef inner;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0;
}
