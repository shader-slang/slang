// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374.
//
// Covers the built-in path, which the other four cases in this family do not:
// they all use user-authored functions, so none of them reaches the
// `IRSpecialize` / `IRGeneric` arms of `isDerivativeAssumedReadNone`, and none
// of them exercises a *built-in* derivative.
//
// `dot` is `[__readNone]` (hlsl.meta.slang) and its derivatives `__d_dot` are
// generic and marked `[PreferRecompute]` (diff.meta.slang) rather than
// `[__readNone]`, so a call to it reaches the gate as a specialize-of-generic
// wrapping a `[PreferRecompute]` function. `__d_dot` is genuinely pure — it
// only reads its `DifferentialPair` arguments — so the derivatives-aware gate
// added for #11374 must still treat `dot(x, x)` as not carrying a derivative
// when its arguments are `no_diff`.
//
// If the gate fails to recognize a pure built-in derivative, this fires a
// spurious E41031 — which is exactly the #11285 false positive that #11286
// fixed, re-opened from the other direction. That makes this the negative
// case that guards the new gate's built-in-purity signal.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

RWStructuredBuffer<float> out_buf;

[Differentiable]
public void caller_builtin(no_diff float3 x, no_diff out float result)
{
    result = dot(x, x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    caller_builtin(float3(1.0, 2.0, 3.0), r);
    out_buf[0] = r;
}
