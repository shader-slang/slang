// Positive test for https://github.com/shader-slang/slang/issues/11374.
//
// PR #11286 added a `[__readNone]` gate to the Call-rule carry-set
// tightening: a differentiable call only enters `carryNonTrivialDiffSet`
// when one of its differentiable-input arguments does. That gate inspects
// only the *primary* callee.
//
// A `[__readNone]` primary whose user-supplied `[BackwardDerivative]` (or
// `[ForwardDerivative]`) is NOT `[__readNone]` can still produce observable
// derivative state through differentiation — the user-supplied derivative
// is free to read or write captured buffers. The carry-set analysis must
// reject the optimization in that case so that storing the call result
// into a `no_diff` slot is diagnosed.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

public struct Sample
{
    public float val;
}

RWStructuredBuffer<float> debug_buf;

// User-supplied backward-derivative with an observable side effect. Not
// `[__readNone]`: the side effect makes the call's derivative state
// dependent on captured buffer state, not only on its arguments.
void foo_bwd(inout DifferentialPair<float> dpx, float.Differential dy)
{
    debug_buf[0] = dpx.p;
    dpx = diffPair(dpx.p, 2 * dpx.p * dy);
}

[__readNone]
[BackwardDerivative(foo_bwd)]
float foo(float x) { return x * x; }

[Differentiable]
public void compute(no_diff float v, no_diff out Sample sample)
{
    sample = {};
    // Without #11374's fix the Call-rule tightening fires (primary is
    // readNone, the single arg is `no_diff` so it doesn't carry), and the
    // call is wrongly classified as not carrying. After the fix, the
    // associated `foo_bwd` is non-readNone so the gate falls back to
    // "differentiable call always carries", which makes this store a real
    // E41031.
    sample.val = foo(v);
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Sample s;
    compute(1.0, s);
    out_buf[0] = s.val;
}
