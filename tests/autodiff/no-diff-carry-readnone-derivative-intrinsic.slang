// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// Neither `[PreferRecompute]` nor `[__target_intrinsic]` is a purity contract:
// `[PreferRecompute]` only requests recomputation (it warns on side effects,
// it does not forbid them), and `[__target_intrinsic]` is user-spellable and
// does not forbid a function body. So a side-effecting backward derivative
// carrying BOTH must NOT be treated as readNone-elidable — storing the call
// result into a no_diff slot must still fire E41031. Only a real `[__readNone]`
// contract lets the carry-set gate trust a derivative variant.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debugBuf;
RWStructuredBuffer<float> out_buf;

// A user-authored backward derivative with side effects, carrying BOTH
// [PreferRecompute] and __target_intrinsic. Neither is a purity contract, so
// the call must still be treated as carrying a derivative.
[PreferRecompute]
__target_intrinsic(hlsl, "debugBuf[0] = ($0).p")
void quux_bwd(inout DifferentialPair<float> v, float dOut)
{
    debugBuf[0] = v.p;
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(quux_bwd)]
[__readNone]
float quux(float x)
{
    return x * x;
}

[Differentiable]
public void caller_intrinsic(no_diff float x, no_diff out float result)
{
    result = quux(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    caller_intrinsic(2.0, r);
    out_buf[0] = r;
}
