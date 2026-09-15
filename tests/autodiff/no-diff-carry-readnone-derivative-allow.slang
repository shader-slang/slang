// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// A derivative variant marked `[PreferRecompute(SideEffectBehavior.Allow)]`
// has explicitly opted OUT of the side-effect-free guarantee — that form
// only suppresses the side-effect warning. The carry-set gate must NOT treat
// such a derivative as readNone-elidable, so storing the call result into a
// no_diff slot must still fire E41031. This guards against the readNone gate
// accepting `[PreferRecompute]` unconditionally as a purity signal.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debug_buf;
RWStructuredBuffer<float> out_buf;

// Backward derivative writes to a buffer and uses
// [PreferRecompute(SideEffectBehavior.Allow)] to silence the side-effect
// warning — so it is NOT side-effect free.
[PreferRecompute(SideEffectBehavior.Allow)]
void baz_bwd(inout DifferentialPair<float> v, float dOut)
{
    debug_buf[0] = v.p;
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(baz_bwd)]
[__readNone]
float baz(float x)
{
    return x * x;
}

[Differentiable]
public void caller_allow(no_diff float x, no_diff out float result)
{
    result = baz(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    caller_allow(2.0, r);
    out_buf[0] = r;
}
