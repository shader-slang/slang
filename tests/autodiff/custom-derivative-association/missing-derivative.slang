//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A function that has only a forward derivative has no backward one, and the reverse, whichever
// attribute declares the derivative. `fwd_diff(f)` and `bwd_diff(f)` look up a member of `f`, so
// asking for the missing derivative reports that the member does not exist.
// `func-extension-missing-derivative.slang` covers the derivatives that a `__func_extension`
// declares.

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

[BackwardDerivative(backwardNamedDerivative)]
float backwardNamed(no_diff float a, float x) { return a * x * x; }

void backwardNamedDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

float backwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[BackwardDerivativeOf(backwardOfPrimal)]
void backwardOfDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var dp = diffPair(2.0, 0.0);

    bwd_diff(forwardNamed)(3.0, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'forwardNamed'.
*/
    bwd_diff(forwardOfPrimal)(3.0, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'forwardOfPrimal'.
*/

    outputBuffer[0] = fwd_diff(backwardNamed)(3.0, diffPair(2.0, 1.0)).d;
//CHECK:              ^^^^^^^^ error E30027
//CHECK:              ^^^^^^^^ 'fwd_diff' is not a member of 'backwardNamed'.
    outputBuffer[1] = fwd_diff(backwardOfPrimal)(3.0, diffPair(2.0, 1.0)).d;
//CHECK:              ^^^^^^^^ error E30027
//CHECK:              ^^^^^^^^ 'fwd_diff' is not a member of 'backwardOfPrimal'.
}
