// Companion to no-diff-carry-readnone-derivative.slang for issue
// shader-slang/slang#11374, covering the built-in derivative path.
//
// `dot` is `[__readNone]` (hlsl.meta.slang), so the call reaches the carry-set
// gate as a specialize-of-generic that `isReadNoneCallee` unwraps to a
// `[__readNone]` function. With `no_diff` arguments, `dot(x, x)` must not carry
// a derivative; a gate that rejects the built-in or its pure derivatives
// reports a spurious E41031 here.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

RWStructuredBuffer<float> outBuf;

[Differentiable]
public void callerBuiltin(no_diff float3 x, no_diff out float result)
{
    result = dot(x, x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerBuiltin(float3(1.0, 2.0, 3.0), r);
    outBuf[0] = r;
}
