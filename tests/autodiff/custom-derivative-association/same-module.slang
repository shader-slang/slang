//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Each attribute that gives a function a derivative within one module, and which derivative each
// way of invoking it runs. Every primal computes `a * x * x`, whose derivative with respect to `x`
// is `2 * a * x`, or 12 at a = 3 and x = 2. Every user-defined derivative instead computes
// `1000 + 2 * a * x`, or 1012, so the output shows whether the user-defined derivative ran.
// `func-extension-same-module.slang` covers the derivatives that a `__func_extension` declares.
//
// A function that has only a forward derivative has no backward one, and the reverse, so the
// invocations that would need the missing one are rejected; `missing-derivative.slang` and
// `caller-of-forward-only.slang` cover those.

// --- A synthesized derivative ---

[Differentiable]
float synthesized(no_diff float a, float x) { return a * x * x; }

// --- A forward derivative only ---

[ForwardDerivative(forwardNamedDerivative)]
float forwardNamed(no_diff float a, float x) { return a * x * x; }

DifferentialPair<float> forwardNamedDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

float forwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[ForwardDerivativeOf(forwardOfPrimal)]
DifferentialPair<float> forwardOfDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

// --- A backward derivative only ---

[BackwardDerivative(backwardNamedDerivative)]
float backwardNamed(no_diff float a, float x) { return a * x * x; }

void backwardNamedDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

float backwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[BackwardDerivativeOf(backwardOfPrimal)]
void backwardOfDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

// --- Differentiable callers, which differentiate the call to the function they wrap ---

[Differentiable]
float callSynthesized(float x) { return synthesized(3.0, x); }

[Differentiable]
float callBackwardNamed(float x) { return backwardNamed(3.0, x); }

[Differentiable]
float callBackwardOfPrimal(float x) { return backwardOfPrimal(3.0, x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A synthesized derivative serves every invocation.
    outputBuffer[0] = fwd_diff(synthesized)(3.0, diffPair(2.0, 1.0)).d;
    var dp0 = diffPair(2.0, 0.0);
    bwd_diff(synthesized)(3.0, dp0, 1.0);
    outputBuffer[1] = dp0.d;
    outputBuffer[2] = fwd_diff(callSynthesized)(diffPair(2.0, 1.0)).d;
    var dp1 = diffPair(2.0, 0.0);
    bwd_diff(callSynthesized)(dp1, 1.0);
    outputBuffer[3] = dp1.d;
    // CHECK: type: float
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0

    // A forward derivative serves `fwd_diff`, whichever attribute declares it.
    outputBuffer[4] = fwd_diff(forwardNamed)(3.0, diffPair(2.0, 1.0)).d;
    outputBuffer[5] = fwd_diff(forwardOfPrimal)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 1012.0

    // A backward derivative serves `bwd_diff` and the backward derivative of a caller, whichever
    // attribute declares it. The forward derivative of a caller treats the call as having no
    // derivative and gives 0 without a diagnostic, unlike a call to a function that has only a
    // forward derivative, which `caller-of-forward-only.slang` shows is rejected.
    var dp2 = diffPair(2.0, 0.0);
    bwd_diff(backwardNamed)(3.0, dp2, 1.0);
    outputBuffer[6] = dp2.d;
    outputBuffer[7] = fwd_diff(callBackwardNamed)(diffPair(2.0, 1.0)).d;
    var dp3 = diffPair(2.0, 0.0);
    bwd_diff(callBackwardNamed)(dp3, 1.0);
    outputBuffer[8] = dp3.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    var dp4 = diffPair(2.0, 0.0);
    bwd_diff(backwardOfPrimal)(3.0, dp4, 1.0);
    outputBuffer[9] = dp4.d;
    outputBuffer[10] = fwd_diff(callBackwardOfPrimal)(diffPair(2.0, 1.0)).d;
    var dp5 = diffPair(2.0, 0.0);
    bwd_diff(callBackwardOfPrimal)(dp5, 1.0);
    outputBuffer[11] = dp5.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0
}
