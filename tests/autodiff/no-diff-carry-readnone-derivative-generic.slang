// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374, covering the built-in derivative path.
//
// `dot` is `[__readNone]` (hlsl.meta.slang) and its generic derivatives
// `__d_dot` are `[__readNone]` (diff.meta.slang), so the call reaches the
// carry-set gate as a specialize-of-generic wrapping a `[__readNone]` function.
// This exercises the specialize-of-generic unwrapping in `isReadNoneCallee`:
// the gate must recognize `__d_dot` as readNone and treat `dot(x, x)` as not
// carrying a derivative when its arguments are `no_diff`.
//
// A gate that fails to unwrap to the pure built-in derivative reports a
// spurious E41031 here.

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
