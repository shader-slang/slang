// A differentiable function with a `groupshared` parameter cannot itself be differentiated, but it
// can still be called without differentiating it: directly, and under `no_diff` from a body that
// autodiff differentiates. Its body may read and write the shared memory in place. The error for
// differentiating such a function is covered by
// tests/diagnostics/groupshared-autodiff/differentiate-groupshared-param.slang.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float gFloats[4];

[Differentiable]
float scaleMarked(uint i, no_diff groupshared float s[4], float x)
{
    s[i] = s[i] + 1.0;
    return s[i] * x;
}

[Differentiable]
float scaleUnmarked(uint i, groupshared float s[4], float x)
{
    return s[i] * x;
}

[Differentiable]
float outer(uint i, float x)
{
    return no_diff(scaleUnmarked(i, gFloats, 1.0)) * x * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    gFloats[0] = 2.0;
    gFloats[1] = 4.0;

    // (s[0] + 1) * x = 3 * 5, and the call writes s[0] once.
    outputBuffer[0] = scaleMarked(0, gFloats, 5.0);
    outputBuffer[1] = gFloats[0];

    // d/dx s[1] * x * x = 2 * 4 * 5 = 40.
    var b = diffPair(5.0, 0.0);
    bwd_diff(outer)(1, b, 1.0);
    outputBuffer[2] = b.d;
    outputBuffer[3] = gFloats[1];
}

// CHECK: 15.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 40.0
// CHECK-NEXT: 4.0
