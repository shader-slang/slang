//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/10081
// When a [Differentiable] function takes an inout struct and reads one field
// to write to another (s.a = s.b), the backward pass must propagate the
// gradient from s.a back to s.b.

struct S : IDifferentiable
{
    float a;
    float b;
}

[Differentiable]
void just_copy_field(inout S s)
{
    s.a = s.b;
}

[Differentiable]
float test(float a, float b)
{
    S s = { a, b };
    just_copy_field(s);
    return s.a;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // d(test)/d(b) should be 1.0 since test returns s.a which equals s.b
    DifferentialPair<float> dpA = diffPair(1.0, 0.0);
    DifferentialPair<float> dpB = diffPair(3.0, 0.0);
    bwd_diff(test)(dpA, dpB, 1.0);
    outputBuffer[0] = dpB.d;
    // CHECK: 1.0

    // d(test)/d(a) should be 0.0 since a is overwritten
    DifferentialPair<float> dpA2 = diffPair(1.0, 0.0);
    DifferentialPair<float> dpB2 = diffPair(3.0, 0.0);
    bwd_diff(test)(dpA2, dpB2, 1.0);
    outputBuffer[1] = dpA2.d;
    // CHECK: 0.0

    // Forward pass: test(1,3) = 3 since s.a = s.b = 3
    outputBuffer[2] = test(1.0, 3.0);
    // CHECK: 3.0
}
