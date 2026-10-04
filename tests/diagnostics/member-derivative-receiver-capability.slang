//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

// The receiver of a derivative contributes its capability requirements, as in a plain call.

struct Scaled : IDifferentiable
{
    float k;

    [Differentiable]
    float eval(float t) { return k * t * t; }
}

[require(cuda)]
Scaled makeScaled()
{
    Scaled s;
    s.k = 5.0;
    return s;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
/*CHECK:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'spirv' compilation target.
*/
{
    var p = diffPair(2.0);
    __bwd_diff(makeScaled().eval)(p, 1.0);
//CHECK:       ^^^^^^^^^^ see using of 'makeScaled'
    outputBuffer[0] = p.d;
}
