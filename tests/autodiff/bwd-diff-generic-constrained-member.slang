// Companion to `fwd-diff-generic-constrained-member.slang` for
// shader-slang/slang#11004: `__bwd_diff` applied to a non-static member
// method on a generic-constrained interface-typed receiver must thread
// the receiver as the implicit `this` argument and back-propagate the
// gradient correctly.
//
// `Helix.eval(t) = 2*t*t  ⇒  ∂eval/∂t = 4*t`, so for
// `__bwd_diff(curve.eval)(pair, dResult)` with `pair = (t, 0)`:
//   pair.d after call = 4*t * dResult
//   t = 2.0, dResult = 1.0  →  pair.d =  8.0
//   t = 3.0, dResult = 1.0  →  pair.d = 12.0

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

interface ICurve : IDifferentiable
{
    [Differentiable]
    float eval(float t);
}

struct Helix : ICurve, IDifferentiable
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t)
    {
        return 2.0 * t * t;
    }
}

[Differentiable]
float gradGeneric<C : ICurve>(C curve, float t, float dResult)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, dResult);
    return pair.d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Helix h;

    // CHECK: 8.0
    outputBuffer[0] = gradGeneric<Helix>(h, 2.0, 1.0);

    // CHECK: 12.0
    outputBuffer[1] = gradGeneric<Helix>(h, 3.0, 1.0);
}
