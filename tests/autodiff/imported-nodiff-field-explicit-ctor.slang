//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Regression for #13233. Forward-mode differentiation of a function whose parameter is an
// `IDifferentiable` struct imported from another module used to mistype the derivative's
// parameter as the primal struct with a hardcoded zero tangent -- but only when the struct
// combined an explicit `__init()` with a `no_diff` field.
//
// The differentiable-type association for the imported `Value` is emitted onto this
// module's `[import]` declaration, while link selects the defining module's `[export]`
// (which lacks the association, because its explicit `__init()` means it never
// differentiates `Value`) as the canonical inst. Before the fix the association was not
// carried across, so `fwd_diff(metric)` fell back to a non-differentiable parameter. We
// check the derivative sees a real `DifferentialPair`: primal `metric(v) = v.x * 2 = 0`
// and tangent `2 * dv.x = 2`.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "imported-nodiff-field-explicit-ctor-lib";

[Differentiable]
float metric(Value v)
{
    return v.x * 2.0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Value v;                     // x = 0
    var dp = diffPair(v, {1.0}); // tangent dx = 1
    var r = fwd_diff(metric)(dp);

    outputBuffer[0] = r.p; // metric(v) = 0
    outputBuffer[1] = r.d; // 2 * dv.x = 2

    // CHECK: 0.0
    // CHECK-NEXT: 2.0
}
