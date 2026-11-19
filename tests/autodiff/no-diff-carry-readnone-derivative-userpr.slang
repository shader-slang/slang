// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// A derivative marked with plain `[PreferRecompute]` (the default
// `SideEffectBehavior.Warn` form) is NOT a purity contract — that form only
// requests recomputation and warns if side effects are detected; it still
// compiles. So the carry-set gate must NOT treat a `[PreferRecompute]`
// derivative as readNone-elidable just because the decoration is present; only
// a real `[__readNone]` contract lets the gate trust a derivative variant.
//
// Here the backward derivative writes to a buffer (has side effects) and
// carries plain `[PreferRecompute]`. Storing the call result into a no_diff
// slot must still fire E41031.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debugBuf;
RWStructuredBuffer<float> outBuf;

// User-authored backward derivative with side effects, marked with the
// default `[PreferRecompute]` (Warn) — NOT side-effect free.
[PreferRecompute]
void quxBwd(inout DifferentialPair<float> v, float dOut)
{
    debugBuf[0] = v.p;
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(quxBwd)]
[__readNone]
float qux(float x)
{
    return x * x;
}

[Differentiable]
public void callerUserPr(no_diff float x, no_diff out float result)
{
    result = qux(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerUserPr(2.0, r);
    outBuf[0] = r;
}
