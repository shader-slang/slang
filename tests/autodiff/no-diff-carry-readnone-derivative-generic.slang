// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374, covering the built-in derivative path.
//
// `dot` is `[__readNone]` (hlsl.meta.slang) and its derivatives `__d_dot` are
// generic and marked `[PreferRecompute]` (diff.meta.slang) rather than
// `[__readNone]`, so the call reaches the carry-set gate as a
// specialize-of-generic wrapping a `[PreferRecompute]` function. `__d_dot` only
// reads its `DifferentialPair` arguments, so the gate must still treat
// `dot(x, x)` as not carrying a derivative when its arguments are `no_diff`.
//
// A gate that fails to recognize a pure built-in derivative reports a spurious
// E41031 here.

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
