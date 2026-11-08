//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

// Each derivative that a `__func_extension` in this module declares for a function from another
// module, and which derivative each way of invoking it runs. As in `same-module.slang`, every
// primal computes `a * x * x`, whose derivative is 12 at a = 3 and x = 2, and every user-defined
// derivative computes 1012 instead.
//
// A backward derivative needs this module to import the function with `__exported import`, as
// for a `[BackwardDerivativeOf]` attribute; `func-extension-non-exported-import.slang` covers the
// error otherwise. A custom forward pass does not. An extension in another module is not visible
// here, so only extensions in this module appear; `func-extension-library.slang` and
// `func-extension-split.slang` cover the others.

import "cross-module-primal";
__exported import "cross-module-exported-primal";

__func_extension fwd_diff(mainForwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

__func_extension fwd_diff(exportedMainForwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

__func_extension bwd_diff(exportedMainBackwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

struct ApplyExtendedContext
{
    float a;
    float x;
    void operator()(__none_t da, out float dx, float dOut) { dx = dOut * (1000.0 + 2.0 * a * x); }
};

__func_extension __apply(mainApplyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

__func_extension __apply(exportedMainApplyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

[Differentiable]
float callExportedMainBackwardExtended(float x) { return exportedMainBackwardExtended(3.0, x); }

[Differentiable]
float callMainApplyExtended(float x) { return mainApplyExtended(3.0, x); }

[Differentiable]
float callExportedMainApplyExtended(float x) { return exportedMainApplyExtended(3.0, x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = fwd_diff(mainForwardExtended)(3.0, diffPair(2.0, 1.0)).d;
    outputBuffer[1] = fwd_diff(exportedMainForwardExtended)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK: type: float
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 1012.0

    var dp0 = diffPair(2.0, 0.0);
    bwd_diff(exportedMainBackwardExtended)(3.0, dp0, 1.0);
    outputBuffer[2] = dp0.d;
    outputBuffer[3] = fwd_diff(callExportedMainBackwardExtended)(diffPair(2.0, 1.0)).d;
    var dp1 = diffPair(2.0, 0.0);
    bwd_diff(callExportedMainBackwardExtended)(dp1, 1.0);
    outputBuffer[4] = dp1.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    // A custom forward pass for a function from a module imported with a plain `import`, which a
    // backward derivative could not use.
    var dp2 = diffPair(2.0, 0.0);
    bwd_diff(mainApplyExtended)(3.0, dp2, 1.0);
    outputBuffer[5] = dp2.d;
    outputBuffer[6] = fwd_diff(callMainApplyExtended)(diffPair(2.0, 1.0)).d;
    var dp3 = diffPair(2.0, 0.0);
    bwd_diff(callMainApplyExtended)(dp3, 1.0);
    outputBuffer[7] = dp3.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    var dp4 = diffPair(2.0, 0.0);
    bwd_diff(exportedMainApplyExtended)(3.0, dp4, 1.0);
    outputBuffer[8] = dp4.d;
    outputBuffer[9] = fwd_diff(callExportedMainApplyExtended)(diffPair(2.0, 1.0)).d;
    var dp5 = diffPair(2.0, 0.0);
    bwd_diff(callExportedMainApplyExtended)(dp5, 1.0);
    outputBuffer[10] = dp5.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0
}
