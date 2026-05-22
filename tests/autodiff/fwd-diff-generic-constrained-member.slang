// Regression test for shader-slang/slang#11004:
// __fwd_diff applied to a member method on a generic-constrained
// interface-typed receiver previously ICEd in propagateConstExpr with
// "paramCount == callArgCount". The fix lowers the differentiated call
// so the operand list and callee `IRFuncType` agree on parameter
// counts even when the callee is the AD 2.0 lookup of `fwd_diff` on a
// function-as-type whose `_resolveImplOverride()` has already
// substituted the receiver into the resolved `FuncType`.
//
// CPU-compute coverage so the test is sensitive to a wrong-receiver
// miscompile (the OpEntryPoint-only check used in the first draft of
// this test would have passed even if `curve` was silently dropped).
// `Helix.eval(t) = 2*t*t   ⇒   eval'(t) = 4*t`, so:
//   tangentGeneric<Helix>(h, 2.0) = 8.0
//   tangentGeneric<Helix>(h, 3.0) = 12.0

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

// Generic-constrained method dispatch — the exact shape from the bug.
[Differentiable]
float tangentGeneric<C : ICurve>(C curve, float t)
{
    let pair = __fwd_diff(curve.eval)(diffPair(t, 1.0));
    return pair.d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Helix h;

    // CHECK: 8.0
    outputBuffer[0] = tangentGeneric<Helix>(h, 2.0);

    // CHECK: 12.0
    outputBuffer[1] = tangentGeneric<Helix>(h, 3.0);
}
