// Companion regression test for shader-slang/slang#11004:
// `__bwd_diff(obj.method)` shares the same AD 2.0 lookup path that
// `__fwd_diff(obj.method)` exercises (the same surfacing of
// `SharedTypeExpr::base.exp` and the same `funcTypeAlreadyHasImplicitThis`
// branch in `visitInvokeExprImpl`), so we mirror the forward-mode
// coverage to keep the reverse-mode half from regressing silently.
//
// For `Helix.eval(t) = 2*t*t`, the reverse-mode signature is
// `eval_bwd(inout DifferentialPair<float> t_pair, float dResult)`,
// and `t_pair.d = 4*t * dResult` after the call:
//   pair = DifferentialPair(2.0, 0); __bwd_diff(curve.eval)(pair, 1.0)
//     → pair.d = 4*2*1 = 8.0
//   pair = DifferentialPair(3.0, 0); __bwd_diff(curve.eval)(pair, 1.0)
//     → pair.d = 4*3*1 = 12.0

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

// Generic-constrained method dispatch — the same shape #11004 fixed
// for the forward-mode case, mirrored for the reverse-mode lookup.
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
