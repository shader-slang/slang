// Regression test for shader-slang/slang#11374. Follow-up to #11286.
//
// PR #11286 narrowed the carry-set decision for [Differentiable] calls so a
// [__readNone] callee whose arguments don't carry a derivative is treated as
// not carrying. That tightening must also consider the side-effect status of
// any associated derivative variants: a primary [__readNone] callee can have
// a user-defined [ForwardDerivative] / [BackwardDerivative] variant that
// writes to memory, and that variant is what runs after the autodiff
// transform. If the gate skips the call based only on the primary's
// readNone-ness, the missing-no_diff diagnostic that #11285 expected to
// fire here is silently suppressed.
//
// This file exercises a custom backward-derivative variant with side
// effects. With the fix in slang-ir-check-differentiability.cpp, the gate
// requires every associated derivative variant to be readNone before
// treating the call as elidable, so E41031 fires correctly.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debug_buf;
RWStructuredBuffer<float> out_buf;

// Backward-mode derivative writes to a buffer — NOT readNone — even though
// the primary `foo` is [__readNone].
void foo_bwd(inout DifferentialPair<float> v, float dOut)
{
    debug_buf[0] = v.p;
    v = diffPair(v.p, dOut * 2.0 * v.p);
}

[BackwardDerivative(foo_bwd)]
[__readNone]
float foo(float x)
{
    return x * x;
}

// `x` is `no_diff` — does NOT carry a derivative. With #11286 alone the
// gate sees `foo` is readNone and `x` doesn't carry, treats `foo(x)` as
// not carrying, and the store into the no_diff `result` slot is silently
// accepted. With #11374 the gate also notices `foo_bwd` is non-readNone,
// falls back to "carries", and the store fires E41031.
[Differentiable]
public void caller(no_diff float x, no_diff out float result)
{
    result = foo(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    caller(2.0, r);
    out_buf[0] = r;
}
