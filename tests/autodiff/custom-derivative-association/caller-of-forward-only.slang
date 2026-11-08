//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A differentiable function that calls a function with only a forward derivative cannot
// propagate a derivative through the call, whichever attribute declares the derivative and
// whichever module declares it, and the error names the missing backward derivative even when
// only the caller's forward derivative is requested. A call to a function with only a backward
// derivative is instead treated as having no forward derivative, as `same-module.slang` shows.
// The error comes from the autodiff passes, so this test has no semantic errors that would stop
// compilation before them. `func-extension-caller-of-forward-only.slang` covers the forward
// derivatives that a `__func_extension` declares.

import "cross-module-library";

[ForwardDerivative(forwardNamedDerivative)]
float forwardNamed(no_diff float a, float x) { return a * x * x; }

DifferentialPair<float> forwardNamedDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

float forwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[ForwardDerivativeOf(forwardOfPrimal)]
DifferentialPair<float> forwardOfDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[Differentiable]
float callForwardNamed(float x) { return forwardNamed(3.0, x); }
//CHECK:                                             ^ error E41022
//CHECK:                                             ^ derivative cannot be propagated through call to non-backward-differentiable function `forwardNamed`, use 'no_diff' to clarify intention.

[Differentiable]
float callForwardOfPrimal(float x) { return forwardOfPrimal(3.0, x); }
//CHECK:                                                   ^ error E41022
//CHECK:                                                   ^ derivative cannot be propagated through call to non-backward-differentiable function `forwardOfPrimal`, use 'no_diff' to clarify intention.

[Differentiable]
float callLibraryForwardNamed(float x) { return libraryForwardNamed(3.0, x); }
//CHECK:                                                           ^ error E41022
//CHECK:                                                           ^ derivative cannot be propagated through call to non-backward-differentiable function `libraryForwardNamed`, use 'no_diff' to clarify intention.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(callForwardNamed)(diffPair(2.0, 1.0)).d;
    var dp = diffPair(2.0, 0.0);
    bwd_diff(callForwardOfPrimal)(dp, 1.0);
    outputBuffer[1] = dp.d;
    outputBuffer[2] = fwd_diff(callLibraryForwardNamed)(diffPair(2.0, 1.0)).d;
}
