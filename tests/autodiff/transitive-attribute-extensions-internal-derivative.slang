// `Operations.restricted` is public, but its `[ForwardDerivative(internalForward)]` derivative is
// internal to the library. A caller that reaches `restricted` through the facade cannot use that
// derivative, just as a caller that imports the library directly cannot, so differentiating
// through the call is an error rather than a silent use of the internal function.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cuda -entry computeMain -stage compute

import "transitive-attribute-extensions-facade";

RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float evaluateRestricted(no_diff Container container, float x)
{
    return container.operations.restricted(x);
//CHECK:                                  ^ error E41022
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Container container = {};
    outputBuffer[0] = fwd_diff(evaluateRestricted)(container, diffPair(2.0, 1.0)).d;
}
