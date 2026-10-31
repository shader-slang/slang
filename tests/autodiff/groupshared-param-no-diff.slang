// A differentiable function with a `groupshared` parameter can be differentiated directly, and
// called under `no_diff` from a body that autodiff differentiates. Its body reads and writes the
// shared memory in place, so each call writes the element once, including the primal call that a
// backward derivative makes.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float gFloats[4];

[Differentiable]
float scale(uint i, no_diff groupshared float s[4], float x)
{
    s[i] = s[i] + 1.0;
    return s[i] * x;
}

[Differentiable]
float outer(uint i, float x)
{
    return no_diff(scale(i, gFloats, 1.0)) * x * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    gFloats[0] = 2.0;
    gFloats[1] = 4.0;
    gFloats[2] = 6.0;

    // (s[0] + 1) * x = 3 * 5, and the call writes s[0] once.
    outputBuffer[0] = scale(0, gFloats, 5.0);
    outputBuffer[1] = gFloats[0];

    // d/dx (s[1] + 1) * x * x = 5 * 2 * 5 = 50.
    var b = diffPair(5.0, 0.0);
    bwd_diff(outer)(1, b, 1.0);
    outputBuffer[2] = b.d;
    outputBuffer[3] = gFloats[1];

    // d/dx (s[2] + 1) * x = 7.
    var c = diffPair(5.0, 0.0);
    bwd_diff(scale)(2, gFloats, c, 1.0);
    outputBuffer[4] = c.d;
    outputBuffer[5] = gFloats[2];

    // d/dx (s[3] + 1) * x = 1 for a forward derivative with dx = 1.
    outputBuffer[6] = fwd_diff(scale)(3, gFloats, diffPair(5.0, 1.0)).d;
}

// CHECK: 15.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 50.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 1.0
