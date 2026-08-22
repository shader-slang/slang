// Negative companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// When BOTH the primary callee and its backward derivative are [__readNone]
// the optimization #11286 introduced still applies: a call whose arguments
// don't carry should not be treated as carrying, and storing the result
// into a no_diff slot must compile clean. This guards against the new
// derivatives-aware gate over-tightening and re-introducing the
// false-positive E41031 that #11286 originally fixed.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

RWStructuredBuffer<float> out_buf;

// Both primary and backward-derivative are [__readNone].
[__readNone]
void bar_bwd(inout DifferentialPair<float> v, float dOut)
{
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(bar_bwd)]
[__readNone]
float bar(float x)
{
    return x * x;
}

[Differentiable]
public void caller_clean(no_diff float x, no_diff out float result)
{
    result = bar(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    caller_clean(2.0, r);
    out_buf[0] = r;
}
