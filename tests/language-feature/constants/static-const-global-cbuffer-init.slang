//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// Regression test for https://github.com/shader-slang/slang/issues/8279
// A `static const` global initialized from a cbuffer/tbuffer member is a runtime
// value and must be diagnosed on all targets, not just SPIR-V.

enum Status
{
    Active = 1
}

cbuffer Options
{
    float Value;
    float3 Vec;
    float Arr[4];
    Status StatusValue;
}

tbuffer TOptions
{
    float TValue;
}

// Arithmetic on a cbuffer member (InvokeExpr) must also be caught.
// CHECK: error[E31226]
static const float VALUE = Value;
//CHECK:           ^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^ initializer of static const global 'VALUE' does not evaluate to a compile-time constant

// CHECK: error[E31226]
static const float VALUE2 = Value * 2.0;
//CHECK:           ^^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^^ initializer of static const global 'VALUE2' does not evaluate to a compile-time constant

// tbuffer member must be caught.
// CHECK: error[E31226]
static const float TVALUE = TValue;
//CHECK:           ^^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^^ initializer of static const global 'TVALUE' does not evaluate to a compile-time constant

// Enum-to-underlying-type coercion (BuiltinCastExpr) must be caught.
// CHECK: error[E31226]
static const int STATUS_ACTIVE = StatusValue;
//CHECK:         ^^^^^^^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:         ^^^^^^^^^^^^^ initializer of static const global 'STATUS_ACTIVE' does not evaluate to a compile-time constant

// Swizzle on a cbuffer member (SwizzleExpr) must be caught.
// CHECK: error[E31226]
static const float2 XY = Vec.xy;
//CHECK:            ^^ static const global initializer must be a compile-time constant
//CHECK:            ^^ initializer of static const global 'XY' does not evaluate to a compile-time constant

// Array subscript on a cbuffer member (IndexExpr base) must be caught.
// CHECK: error[E31226]
static const float ELEM = Arr[0];
//CHECK:           ^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^ initializer of static const global 'ELEM' does not evaluate to a compile-time constant

// Vector subscript on a cbuffer member (IndexExpr base) must be caught.
// CHECK: error[E31226]
static const float VELEM = Vec[1];
//CHECK:           ^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^ initializer of static const global 'VELEM' does not evaluate to a compile-time constant

RWStructuredBuffer<float> OutData;

[shader("compute")]
[numthreads(1, 1, 1)]
void ComputeMain(uint3 thread_id : SV_DispatchThreadID)
{
    OutData[thread_id.x] = thread_id.y + VALUE;
}
