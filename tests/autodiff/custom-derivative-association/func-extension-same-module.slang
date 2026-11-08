//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

// Each derivative that a `__func_extension` declares within one module, and which derivative each
// way of invoking it runs. In a `__func_extension`, `fwd_diff(f)`, `bwd_diff(f)` or `__apply(f)`
// names the derivative of `f` that the body declares; it does not invoke one. As in
// `same-module.slang`, every primal computes `a * x * x`, whose derivative is 12 at a = 3 and
// x = 2, and every user-defined derivative computes 1012 instead.

// --- A forward derivative ---

float forwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension fwd_diff(forwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

// --- A backward derivative ---

float backwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension bwd_diff(backwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

// --- A custom forward pass ---

// A custom forward pass returns the primal result together with a context that computes the
// derivative. The context takes `__none_t` in place of the derivative of `a`, which has none.
float applyExtended(no_diff float a, float x) { return a * x * x; }

struct ApplyExtendedContext
{
    float a;
    float x;
    void operator()(__none_t da, out float dx, float dOut) { dx = dOut * (1000.0 + 2.0 * a * x); }
};

__func_extension __apply(applyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

// --- Differentiable callers, which differentiate the call to the function they wrap ---

[Differentiable]
float callBackwardExtended(float x) { return backwardExtended(3.0, x); }

[Differentiable]
float callApplyExtended(float x) { return applyExtended(3.0, x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = fwd_diff(forwardExtended)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK: type: float
    // CHECK-NEXT: 1012.0

    // A backward derivative and a custom forward pass both serve `bwd_diff` and the backward
    // derivative of a caller. As for one that an attribute declares, the forward derivative of a
    // caller gives 0 without a diagnostic.
    var dp0 = diffPair(2.0, 0.0);
    bwd_diff(backwardExtended)(3.0, dp0, 1.0);
    outputBuffer[1] = dp0.d;
    outputBuffer[2] = fwd_diff(callBackwardExtended)(diffPair(2.0, 1.0)).d;
    var dp1 = diffPair(2.0, 0.0);
    bwd_diff(callBackwardExtended)(dp1, 1.0);
    outputBuffer[3] = dp1.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    var dp2 = diffPair(2.0, 0.0);
    bwd_diff(applyExtended)(3.0, dp2, 1.0);
    outputBuffer[4] = dp2.d;
    outputBuffer[5] = fwd_diff(callApplyExtended)(diffPair(2.0, 1.0)).d;
    var dp3 = diffPair(2.0, 0.0);
    bwd_diff(callApplyExtended)(dp3, 1.0);
    outputBuffer[6] = dp3.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0
}
