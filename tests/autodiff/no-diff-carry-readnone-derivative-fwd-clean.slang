// Negative companion to no-diff-carry-readnone-derivative-fwd.slang for #11374.
//
// When both the primary callee and its `[ForwardDerivative]` variant are
// `[__readNone]`, the Call-rule carry-set tightening introduced in #11286
// must still apply on the forward-mode path: a call whose
// differentiable-input arguments do not carry is itself not in
// `carryNonTrivialDiffSet`. This protects the `[ForwardDerivative]` arm of
// the gate added in #11374 from over-rejecting the optimization for
// callees whose forward derivatives are genuinely side-effect-free.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

public struct Sample
{
    public float val;
}

// Forward-derivative with no side effects; explicitly `[__readNone]`.
[__readNone]
DifferentialPair<float> foo_fwd_clean(DifferentialPair<float> dpx)
{
    return DifferentialPair<float>(dpx.p * dpx.p, 2 * dpx.p * dpx.d);
}

[__readNone]
[ForwardDerivative(foo_fwd_clean)]
float foo_clean(float x) { return x * x; }

[ForwardDifferentiable]
public void compute_clean(no_diff float v, no_diff out Sample sample)
{
    sample = {};
    // Primary and forward-derivative are both `[__readNone]`. The gate
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
