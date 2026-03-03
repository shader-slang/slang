// The __ref and __constref modifiers are parameter-only qualifiers.
// When applied to struct fields, the compiler rejects them with error 31201
// ("modifier not allowed here"). This holds regardless of whether the field
// type is an interface or a concrete type, but the interface case is
// particularly important because storing a reference to an AnyValue-packed
// existential tuple in a struct field would bypass the pack/unpack
// marshalling required for dynamic dispatch.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain

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

// Direct __ref to interface type on struct field.
struct RefFieldDirect
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __ref IFoo field;
}

// Direct __constref to interface type on struct field.
struct ConstrefFieldDirect
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __constref IFoo field;
}

// __ref on a concrete type struct field (same restriction applies).
struct RefFieldConcrete
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __ref float field;
}

// __constref on a concrete type struct field.
struct ConstrefFieldConcrete
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __constref float field;
}

// Both __ref and __constref on fields in the same struct.
struct MixedRefFields
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __ref IFoo refField;
    // CHECK: ([[# @LINE+1]]): error 31201
    __constref IFoo constrefField;
}

// Nested struct: inner struct tries to use __ref on interface field.
struct Inner
{
    // CHECK: ([[# @LINE+1]]): error 31201
    __ref IFoo iface;
}

struct Outer
{
    Inner inner;
    float value;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0;
}
