// The __ref and __constref modifiers are parameter-only qualifiers.
// When applied to struct fields, the compiler rejects them with error 31201
// ("modifier not allowed here"). This holds regardless of whether the field
// type is an interface or a concrete type, but the interface case is
// particularly important because storing a reference to an AnyValue-packed
// existential tuple in a struct field would bypass the pack/unpack
// marshalling required for dynamic dispatch.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -stage compute -entry computeMain

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
    __ref IFoo field;
/*CHECK:
    ^^^^^ modifier not allowed
    ^^^^^ modifier '__ref' is not allowed here.
*/
}

// Direct __constref to interface type on struct field.
struct ConstrefFieldDirect
{
    __constref IFoo field;
/*CHECK:
    ^^^^^^^^^^ modifier not allowed
    ^^^^^^^^^^ modifier '__constref' is not allowed here.
*/
}

// __ref on a concrete type struct field (same restriction applies).
struct RefFieldConcrete
{
    __ref float field;
/*CHECK:
    ^^^^^ modifier not allowed
    ^^^^^ modifier '__ref' is not allowed here.
*/
}

// __constref on a concrete type struct field.
struct ConstrefFieldConcrete
{
    __constref float field;
/*CHECK:
    ^^^^^^^^^^ modifier not allowed
    ^^^^^^^^^^ modifier '__constref' is not allowed here.
*/
}

// Both __ref and __constref on fields in the same struct.
struct MixedRefFields
{
    __ref IFoo refField;
/*CHECK:
    ^^^^^ modifier not allowed
    ^^^^^ modifier '__ref' is not allowed here.
*/
    __constref IFoo constrefField;
/*CHECK:
    ^^^^^^^^^^ modifier not allowed
    ^^^^^^^^^^ modifier '__constref' is not allowed here.
*/
}

// Nested struct: inner struct tries to use __ref on interface field.
struct Inner
{
    __ref IFoo iface;
/*CHECK:
    ^^^^^ modifier not allowed
    ^^^^^ modifier '__ref' is not allowed here.
*/
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
