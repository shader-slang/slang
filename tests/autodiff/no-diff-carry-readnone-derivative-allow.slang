// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// `[PreferRecompute(SideEffectBehavior.Allow)]` only suppresses the
// side-effect warning; like the default form, it says nothing about purity.
// A side-effecting derivative carrying it must still make the call carry, so
// storing the call result into a no_diff slot fires E41031. This guards
// against the carry-set gate treating `[PreferRecompute]` in any form as a
// purity signal.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debugBuf;
RWStructuredBuffer<float> outBuf;

// Backward derivative writes to a buffer and uses
// [PreferRecompute(SideEffectBehavior.Allow)] to silence the side-effect
// warning — so it is NOT side-effect free.
[PreferRecompute(SideEffectBehavior.Allow)]
void bazBwd(inout DifferentialPair<float> v, float dOut)
{
    debugBuf[0] = v.p;
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(bazBwd)]
[__readNone]
float baz(float x)
{
    return x * x;
}

[Differentiable]
public void callerAllow(no_diff float x, no_diff out float result)
{
    result = baz(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerAllow(2.0, r);
    outBuf[0] = r;
}
