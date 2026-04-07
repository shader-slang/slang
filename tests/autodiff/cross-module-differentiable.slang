//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Test that a [Differentiable] function imported from another module
// can be used inside a differentiable function in this module.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import cross_module_differentiable_lib;

// Use the imported differentiable function inside our own differentiable function.
[Differentiable]
float scaledLength(float3 v, float s)
{
    return s * squaredLength(v);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // v = (1, 2, 3), s = 2
    // scaledLength = s * dot(v,v) = 2 * 14 = 28
    // d/ds scaledLength = dot(v,v) = 14

    float3 v = float3(1.0, 2.0, 3.0);
    DifferentialPair<float3> dpv = diffPair(v, float3(0.0));
    DifferentialPair<float> dps = diffPair(2.0, 1.0);

    float result = fwd_diff(scaledLength)(dpv, dps).d;

    // d/ds of s*dot(v,v) with dv=0, ds=1 => dot(v,v) = 1+4+9 = 14
    outputBuffer[0] = result;

    // Now differentiate w.r.t. v with s fixed.
    // d(s*dot(v,v))/dv_x = 2*s*v_x = 2*2*1 = 4
    dpv = diffPair(v, float3(1.0, 0.0, 0.0));
    dps = diffPair(2.0, 0.0);
    outputBuffer[1] = fwd_diff(scaledLength)(dpv, dps).d;

    // d/dv_y = 2*s*v_y = 2*2*2 = 8
    dpv = diffPair(v, float3(0.0, 1.0, 0.0));
    outputBuffer[2] = fwd_diff(scaledLength)(dpv, dps).d;

    // d/dv_z = 2*s*v_z = 2*2*3 = 12
    dpv = diffPair(v, float3(0.0, 0.0, 1.0));
    outputBuffer[3] = fwd_diff(scaledLength)(dpv, dps).d;

    // CHECK:      14.0
    // CHECK-NEXT: 4.0
    // CHECK-NEXT: 8.0
    // CHECK-NEXT: 12.0
}
