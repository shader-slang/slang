//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A differentiable function that calls a function to which a `__func_extension` gives only a
// forward derivative cannot propagate a derivative through the call, as in
// `caller-of-forward-only.slang`.

import "cross-module-extension-library";

float forwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension fwd_diff(forwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[Differentiable]
float callForwardExtended(float x) { return forwardExtended(3.0, x); }
//CHECK:                                                   ^ error E41022
//CHECK:                                                   ^ derivative cannot be propagated through call to non-backward-differentiable function `forwardExtended`, use 'no_diff' to clarify intention.

// An extension in another module is not visible here, so the function it extends has no
// derivative at all, and the error is the same.
[Differentiable]
float callLibraryForwardExtended(float x) { return libraryForwardExtended(3.0, x); }
//CHECK:                                                                 ^ error E41022
//CHECK:                                                                 ^ derivative cannot be propagated through call to non-backward-differentiable function `libraryForwardExtended`, use 'no_diff' to clarify intention.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(callForwardExtended)(diffPair(2.0, 1.0)).d;
    var dp = diffPair(2.0, 0.0);
    bwd_diff(callLibraryForwardExtended)(dp, 1.0);
    outputBuffer[1] = dp.d;
}
