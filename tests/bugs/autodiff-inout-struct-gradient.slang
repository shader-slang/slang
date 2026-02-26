// Regression test: autodiff inout struct cross-field gradient behavior
// Issue: https://github.com/shader-slang/slang/issues/10081
// Expected: output[0] == 0.0 (maintainer-accepted behavior for d(test)/db)

struct S {
    float a;
    float b;
};

[Differentiable]
void just_copy_field(inout S s) {
    s.a = s.b;
}

[Differentiable]
float test(float a, float b) {
    S s = { a, b };
    just_copy_field(s);
    return s.a;
}

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    float a_val = 1.0;
    float b_val = 3.0;

    DifferentialPair<float> dpA = diffPair(a_val, 0.0);
    DifferentialPair<float> dpB = diffPair(b_val, 0.0);

    bwd_diff(test)(dpA, dpB, 1.0);

    // Write gradient to output buffer for validation.
    // Expected: 0.0 (maintainer-accepted behavior).
    output[0] = dpB.d;
}

// CHECK: 0
