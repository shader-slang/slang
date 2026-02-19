// Bug Report: Autodiff inout struct cross-field gradient is zero
// Issue: https://github.com/shader-slang/slang/issues/10081
// Type: WRONG_OUTPUT
// Expected: output[0] == 1.0 (gradient of d(test)/db)
// Actual:   output[0] == 0.0 (gradient is lost)

//TEST_IGNORE_FILE:

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

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    float a_val = 1.0;
    float b_val = 3.0;

    DifferentialPair<float> dpA = diffPair(a_val, 0.0);
    DifferentialPair<float> dpB = diffPair(b_val, 0.0);

    bwd_diff(test)(dpA, dpB, 1.0);

    // Write gradient to output buffer for validation
    // Expected: 1.0, Actual: 0.0 (BUG)
    output[0] = dpB.d;
}

// CHECK: 1.0
