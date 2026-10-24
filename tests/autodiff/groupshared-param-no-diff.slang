// A `groupshared` parameter of a differentiable function is treated as `no_diff`: the backward
// derivative passes the shared memory through by reference and propagates gradients only to the
// differentiable value parameters. The unmarked spelling behaves the same way after warning.

// The warning for the unmarked spelling is covered by
// tests/diagnostics/reference-param-differentiable.slang, and the warning for passing shared
// memory to a differentiated call by
// tests/diagnostics/groupshared-argument-copied-by-differentiable-call.slang.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -Xslang -warnings-disable -Xslang 38038,38049
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -Xslang -warnings-disable -Xslang 38038,38049

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float gFloats[4];
groupshared uint gUInts[4];

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
float scaleByCount(uint i, groupshared uint n[4], float x)
{
    return float(n[i]) * x * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    gFloats[0] = 2.0;
    gFloats[1] = 4.0;
    gUInts[0] = 3;

    // d/dx (s[0] + 1) * x = 3, and the derivative call runs the write to s[0] once.
    var a = diffPair(5.0, 0.0);
    bwd_diff(scaleMarked)(0, gFloats, a, 1.0);
    outputBuffer[0] = a.d;
    outputBuffer[1] = gFloats[0];

    // d/dx s[1] * x = 4.
    var b = diffPair(5.0, 0.0);
    bwd_diff(scaleUnmarked)(1, gFloats, b, 1.0);
    outputBuffer[2] = b.d;
    outputBuffer[3] = gFloats[1];

    // d/dx n[0] * x * x = 2 * 3 * 5 = 30.
    var c = diffPair(5.0, 0.0);
    bwd_diff(scaleByCount)(0, gUInts, c, 1.0);
    outputBuffer[4] = c.d;
    outputBuffer[5] = float(gUInts[0]);
}

// CHECK: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 30.0
// CHECK-NEXT: 3.0
