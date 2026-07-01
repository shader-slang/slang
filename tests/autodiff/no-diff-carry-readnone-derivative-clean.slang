// Negative companion to no-diff-carry-readnone-derivative.slang for #11374.
//
// When both the primary callee and its `[BackwardDerivative]` variant are
// `[__readNone]`, the Call-rule carry-set tightening introduced in #11286
// must still apply: a call whose differentiable-input arguments do not
// carry is itself not in `carryNonTrivialDiffSet`. This protects the
// stronger gate added in #11374 from over-rejecting the optimization for
// callees whose derivatives are genuinely side-effect-free.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

public struct Sample
{
    public float val;
}

// Backward-derivative with no side effects; explicitly `[__readNone]`.
[__readNone]
void foo_bwd_clean(inout DifferentialPair<float> dpx, float.Differential dy)
{
    dpx = diffPair(dpx.p, 2 * dpx.p * dy);
}

[__readNone]
[BackwardDerivative(foo_bwd_clean)]
float foo_clean(float x) { return x * x; }

[Differentiable]
public void compute_clean(no_diff float v, no_diff out Sample sample)
{
    sample = {};
    // Primary and backward-derivative are both `[__readNone]`. The gate
    // succeeds, the single arg is `no_diff` so it doesn't carry, and the
    // call is correctly classified as not carrying. Storing into the
    // `no_diff` field is fine.
    sample.val = foo_clean(v);
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Sample s;
    compute_clean(1.0, s);
    out_buf[0] = s.val;
}
