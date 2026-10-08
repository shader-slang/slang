// Regression test: forward-differentiating a [TreatAsDifferentiable] function must
// preserve the *primal* value f(x). The synthesized trivial forward-derivative used
// to default-construct (zero) the primal, which collapsed both the primal and any
// surrounding derivative to zero whenever a value flowed through such a function.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Stand-in for a texture lookup: differentiable for type-checking, but its own
// derivative is deliberately ignored.
[TreatAsDifferentiable]
float tex(float u)
{
    return 2.0 * u + 1.0;
}

// tex() is treated as a constant w.r.t. autodiff, but the surrounding `/ x` is a
// genuine differentiable op whose derivative must survive.
[Differentiable]
float divAfterTex(float x)
{
    float s = tex(x);   // primal must be tex(x), differential is 0
    return s / x;
}

// Same, but through a dynamic interface dispatch (mirrors the real-world case where
// the call goes through a `dyn interface` witness).
interface IFoo
{
    [Differentiable]
    float f(float v);
}

struct Tex : IFoo
{
    [TreatAsDifferentiable]
    float f(float v) { return 2.0 * v + 1.0; }
}

[Differentiable]
float divAfterTexIface(IFoo o, float x)
{
    float s = o.f(x);
    return s / x;
}

// A trivial derivative must also force `inout` tangents to zero: the call's
// contribution is df/dx == 0, so the modified value's differential becomes zero
// rather than threading the incoming tangent through unchanged.
[TreatAsDifferentiable]
void scaleInout(inout float x)
{
    x = 3.0 * x;
}

[Differentiable]
float useInout(float x)
{
    float v = x;       // v = (2, 1)
    scaleInout(v);     // primal -> 6; tangent must be forced to 0
    return v;
}

// A pure `out` parameter exercises a different slot-init path in
// translateFuncParam than `inout`: the primal slot is default-constructed
// (not loaded from the caller) and then overwritten by the primal call. The
// callee's computed output must reach the primal — a non-zero value here would
// collapse to 0 under the old bug — while the trivial derivative forces the
// tangent to zero.
[TreatAsDifferentiable]
void produceOut(float x, out float r)
{
    r = 5.0 * x + 1.0;
}

[Differentiable]
float useOut(float x)
{
    float r;
    produceOut(x, r);  // primal r must be 5*x+1, tangent forced to 0
    return r;
}

// A non-differentiable return type (int) takes the final `else` arm of
// generateTrivialFwdDiffFunc: the primal call's result is returned directly
// rather than wrapped in a differential pair. The real classification must be
// returned, not a default/zeroed int.
[TreatAsDifferentiable]
int classify(float x)
{
    return x > 0 ? 1 : -1;
}

[Differentiable]
float useClassify(float x)
{
    int c = classify(x);  // primal must be the real classification, not 0
    return float(c);
}

// A non-differentiable `inout` parameter (`no_diff inout int`) threaded through a
// [TreatAsDifferentiable] callee. The trivial forward body calls the primal
// directly and only writes back the entries recorded for differential-pair
// parameters, so the primal call's mutation of `counter` must survive untouched —
// the pre-fix path explicitly default-constructed (zeroed) *every* out/inout slot,
// which would have clobbered this to 0. The differential must not perturb it.
[TreatAsDifferentiable]
void bumpCounter(no_diff inout int counter, float x)
{
    counter += 1;
}

[Differentiable]
float useCounter(float x)
{
    int n = 2;
    bumpCounter(n, x);  // primal n must be 3 (was 0 under the old default-construct)
    return float(n);
}

// A differential zero is defined by dzero(), not by bitwise/default initialization.
// Using a non-zero representation for the additive identity makes this observable:
// the trivial derivative's inout write-back must produce 42 rather than raw zero.
struct CustomZeroFloat : IDifferentiable
{
    typealias Differential = CustomZeroFloat;

    [DerivativeMember(Differential.value)]
    float value;

    [Differentiable]
    static Differential dzero()
    {
        return { 42.0 };
    }
}

[TreatAsDifferentiable]
void overwriteCustomZeroFloat(inout CustomZeroFloat value)
{
    value.value = 7.0;
}

// The other entry into the trivial-forward synthesis: a function that is
// backward-differentiable ([BackwardDerivative]) but has *no* forward derivative.
// Forward-diffing it hits the BackwardDerivativeApply branch in translateCall
// (distinct from the [TreatAsDifferentiable] branch above), which also synthesizes
// a trivial forward derivative. Its primal must still be square(x), with a zero
// differential.
void square_bwd(inout DifferentialPair<float> dpx, float dy)
{
    dpx = diffPair(dpx.p, 2.0 * dpx.p * dy);
}

[BackwardDerivative(square_bwd)]
float square(float x)
{
    return x * x;
}

[Differentiable]
float divAfterSquare(float x)
{
    float s = square(x);   // primal must be square(x) == 4, differential is 0
    return 10.0 / s;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // f(2) = 5, so divAfterTex(2) = 5/2 = 2.5
    // d/dx (tex/x) with tex treated constant = -tex/x^2 = -5/4 = -1.25
    var dp = __fwd_diff(divAfterTex)(diffPair(2.0, 1.0));

    Tex t;
    var dpIface = __fwd_diff(divAfterTexIface)(t, diffPair(2.0, 1.0));

    // primal preserved (6), inout tangent forced to zero
    var dpInout = __fwd_diff(useInout)(diffPair(2.0, 1.0));

    // square(2) = 4, so divAfterSquare(2) = 10/4 = 2.5
    // square's forward derivative is trivial (df == 0), so d/dx (10/square) = 0
    var dpBwd = __fwd_diff(divAfterSquare)(diffPair(2.0, 1.0));

    // out param: produceOut(2) writes r = 11; primal preserved, tangent forced to 0
    var dpOut = __fwd_diff(useOut)(diffPair(2.0, 1.0));

    // non-diff-pair return: classify(0.5) == 1; primal returned directly
    var dpClassify = __fwd_diff(useClassify)(diffPair(0.5, 1.0));

    // non-diff inout: bumpCounter increments n 2 -> 3; primal preserved (not zeroed),
    // tangent forced to 0
    var dpCounter = __fwd_diff(useCounter)(diffPair(2.0, 1.0));

    // CustomZeroFloat represents its differential zero as 42, so the trivial derivative
    // must call CustomZeroFloat.dzero() rather than raw-defaulting the differential to 0.
    CustomZeroFloat customZeroValue = { 1.0 };
    CustomZeroFloat customZeroTangent = { 5.0 };
    var dpCustomZero = diffPair(customZeroValue, customZeroTangent);
    __fwd_diff(overwriteCustomZeroFloat)(dpCustomZero);

    outputBuffer[0] = dp.p;       // CHECK: 2.5
    outputBuffer[1] = dp.d;       // CHECK: -1.25
    outputBuffer[2] = dpIface.p;  // CHECK: 2.5
    outputBuffer[3] = dpIface.d;  // CHECK: -1.25
    outputBuffer[4] = dpInout.p;  // CHECK: 6.0
    outputBuffer[5] = dpInout.d;  // CHECK: 0.0
    outputBuffer[6] = dpBwd.p;    // CHECK: 2.5
    outputBuffer[7] = dpBwd.d;    // CHECK: 0.0
    outputBuffer[8] = dpOut.p;    // CHECK: 11.0
    outputBuffer[9] = dpOut.d;    // CHECK: 0.0
    outputBuffer[10] = dpClassify.p; // CHECK: 1.0
    outputBuffer[11] = dpCounter.p;  // CHECK: 3.0
    outputBuffer[12] = dpCounter.d;  // CHECK: 0.0
    outputBuffer[13] = dpCustomZero.p.value; // CHECK: 7.0
    outputBuffer[14] = dpCustomZero.d.value; // CHECK: 42.0
}
