// Positive companion to no-diff-carry-readnone-derivative.slang for #11374.
//
// Exercises the `[ForwardDerivative]` arm of `isReadNoneCalleeAndAllDerivatives`
// from a `[ForwardDifferentiable]` caller — logically distinct from the
// backward arm covered by no-diff-carry-readnone-derivative.slang. A
// `[__readNone]` primary whose user-supplied `[ForwardDerivative]` is NOT
// `[__readNone]` must still cause its calls to be classified as carrying so
// that storing the result into a `no_diff` slot is diagnosed.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

public struct Sample
{
    public float val;
}

RWStructuredBuffer<float> debug_buf;

// User-supplied forward-derivative with an observable side effect. Not
// `[__readNone]`: the side effect makes the call's derivative state
// dependent on captured buffer state, not only on its arguments.
DifferentialPair<float> foo_fwd(DifferentialPair<float> dpx)
{
    debug_buf[0] = dpx.p;
    return DifferentialPair<float>(dpx.p * dpx.p, 2 * dpx.p * dpx.d);
}

[__readNone]
[ForwardDerivative(foo_fwd)]
float foo(float x) { return x * x; }

[ForwardDifferentiable]
public void compute(no_diff float v, no_diff out Sample sample)
{
    sample = {};
    // foo is `[__readNone]` but foo_fwd writes to debug_buf. After
    // differentiation the forward derivative's side effect is observable,
    // so the gate must reject the carry-set tightening for this call.
    // Without #11374 the call is wrongly classified as not carrying and
    // the no_diff store is silently accepted.
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
