// A reference parameter of a differentiable function is treated as `no_diff` (warning 38038), so
// differentiating the function propagates derivatives only through its other parameters. Before,
// backward differentiation aborted with "ref parameter not allowed in backward diff function".

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature -Xslang -warnings-disable -Xslang 38038

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float scaleRef(__ref float a, float x)
{
    return a * x;
}

[Differentiable]
float scaleReadOnly(__ref_readonly float a, float x)
{
    return a * x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = 3.0;

    // d(a * x)/dx = a
    var x = diffPair(2.0);
    bwd_diff(scaleRef)(a, x, 1.0);
    outputBuffer[0] = x.d;

    var y = diffPair(5.0);
    bwd_diff(scaleReadOnly)(a, y, 1.0);
    outputBuffer[1] = y.d;

    outputBuffer[2] = fwd_diff(scaleRef)(a, diffPair(2.0, 1.0)).d;

    // The reference parameter is read, not written.
    outputBuffer[3] = a;
}

// CHECK: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
