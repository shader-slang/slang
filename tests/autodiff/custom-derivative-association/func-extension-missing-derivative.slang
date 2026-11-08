//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A function that a `__func_extension` gives only a forward derivative has no backward one, and
// the reverse, as in `missing-derivative.slang`. A custom forward pass gives a function only a
// backward derivative.

float forwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension fwd_diff(forwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

float backwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension bwd_diff(backwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

float applyExtended(no_diff float a, float x) { return a * x * x; }

struct ApplyExtendedContext
{
    float a;
    float x;
    void operator()(__none_t da, out float dx, float dOut) { dx = dOut * 2.0 * a * x; }
};

__func_extension __apply(applyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var dp = diffPair(2.0, 0.0);

    bwd_diff(forwardExtended)(3.0, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'forwardExtended'.
*/

    outputBuffer[0] = fwd_diff(backwardExtended)(3.0, diffPair(2.0, 1.0)).d;
//CHECK:              ^^^^^^^^ error E30027
//CHECK:              ^^^^^^^^ 'fwd_diff' is not a member of 'backwardExtended'.
    outputBuffer[1] = fwd_diff(applyExtended)(3.0, diffPair(2.0, 1.0)).d;
//CHECK:              ^^^^^^^^ error E30027
//CHECK:              ^^^^^^^^ 'fwd_diff' is not a member of 'applyExtended'.
}
