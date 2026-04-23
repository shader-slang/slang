// Test that a `static const` global initialized from a cbuffer member produces a
// diagnostic on all targets, rather than crashing (spirv) or silently succeeding
// (glsl/hlsl).
// Regression test for https://github.com/shader-slang/slang/issues/8279
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv -DNEGATIVE

#ifndef NEGATIVE

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

// CHECK: error[E30305]
static const float VALUE = Value;

// Arithmetic on a cbuffer member should also be caught (InvokeExpr).
// CHECK: error[E30305]
static const float VALUE2 = Value * 2.0;

// tbuffer member reference should also be caught.
tbuffer TOptions
{
    float TValue;
}

// CHECK: error[E30305]
static const float TVALUE = TValue;

// Enum-to-underlying-type coercion uses BuiltinCastExpr (not InvokeExpr) — must be caught.
// CHECK: error[E30305]
static const int STATUS_ACTIVE = StatusValue;

// Swizzle on a cbuffer member (SwizzleExpr) must be caught.
// CHECK: error[E30305]
static const float2 XY = Vec.xy;

// Array/vector subscript on a cbuffer member (IndexExpr) must be caught.
// CHECK: error[E30305]
static const float ELEM = Arr[0];
// CHECK: error[E30305]
static const float VELEM = Vec[1];

RWStructuredBuffer<float> OutData;

[shader("compute")]
[numthreads(1, 1, 1)]
void ComputeMain(uint3 thread_id : SV_DispatchThreadID)
{
    OutData[thread_id.x] = thread_id.y + VALUE;
}

#else

// Negative: static const initialized from a literal or another static const is fine.
// CHECK_NEG-NOT: error[E30305]

static const float PI = 3.14159;
static const float TAU = PI * 2.0;

RWStructuredBuffer<float> OutData;

[shader("compute")]
[numthreads(1, 1, 1)]
void ComputeMain(uint3 thread_id : SV_DispatchThreadID)
{
    OutData[thread_id.x] = TAU;
}

#endif
