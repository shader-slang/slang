// Using Ref<T> as a struct field type is not supported. Reference types
// (Ref, RefParam, BorrowInParam) are designed for function parameters
// and return values; they cannot be stored in struct fields because
// there is no stable storage address to reference across struct copies.

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

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
}

// Ref<IFoo> as a struct field: should be diagnosed.
struct RefInterfaceField
{
    Ref<IFoo> field;
//CHECK:      ^^^^^ reference type cannot be used as struct field
//CHECK:      ^^^^^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// Ref<ConcreteType> as a struct field: same underlying issue.
struct RefConcreteField
{
    Ref<float> field;
//CHECK:       ^^^^^ reference type cannot be used as struct field
//CHECK:       ^^^^^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// Nested: struct containing another struct with Ref<IFoo> field.
struct InnerRef
{
    Ref<IFoo> iface;
//CHECK:      ^^^^^ reference type cannot be used as struct field
//CHECK:      ^^^^^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

struct OuterRef
{
    InnerRef inner;
}

// RefParam<T> as a struct field: also rejected.
struct RefParamField
{
    RefParam<float> p;
//CHECK:            ^ reference type cannot be used as struct field
//CHECK:            ^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// BorrowInParam<T> as a struct field: also rejected.
struct BorrowInParamField
{
    BorrowInParam<float> p;
//CHECK:                 ^ reference type cannot be used as struct field
//CHECK:                 ^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// OutParam<T> as a struct field: also rejected.
struct OutParamField
{
    OutParam<float> p;
//CHECK:            ^ reference type cannot be used as struct field
//CHECK:            ^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// BorrowInOutParam<T> as a struct field: also rejected.
struct BorrowInOutParamField
{
    BorrowInOutParam<float> p;
//CHECK:                    ^ reference type cannot be used as struct field
//CHECK:                    ^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

// Ref<T> in a class field: same restriction applies to all aggregate types.
class ClassWithRefField
{
    Ref<float> field;
//CHECK:       ^^^^^ reference type cannot be used as struct field
//CHECK:       ^^^^^ cannot be used as a struct field; reference types can only be used for function parameters and return values.
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0;
}
