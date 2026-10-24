//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -disable-non-essential-validations

// Reverse mode cannot differentiate a call to a function that is only forward differentiable.
// The differentiability checker reports this, and so does reverse unzipping when that checker is
// disabled, rather than reading a backward derivative that does not exist.

[ForwardDifferentiable]
float forwardOnly(float x) { return x * x; }

[Differentiable]
float use(float x)
{
    return forwardOnly(x) + x;
//CHECK:              ^ derivative cannot be propagated through non-differentiable call
//CHECK:              ^ derivative cannot be propagated through call to non-backward-differentiable function `forwardOnly`, use 'no_diff' to clarify intention.
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    var p = diffPair(3.0);
    __bwd_diff(use)(p, 1.0);
    outputBuffer[0] = p.d;
}
