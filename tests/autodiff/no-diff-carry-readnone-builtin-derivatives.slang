// Calls to `[__readNone]` built-ins whose derivatives are registered in
// diff.meta.slang with `[ForwardDerivativeOf]` / `[BackwardDerivativeOf]`
// must stay non-carrying when every argument is `no_diff`, so storing the
// result into a `no_diff` slot compiles clean. `reflect`, `refract` and
// `determinant` have derivatives that are pure but not marked `[__readNone]`,
// so a gate that looks up the derivatives of the call-site `IRSpecialize`
// reports a spurious E41031 here.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK: void main(

RWStructuredBuffer<float> outBuf;

[Differentiable]
void callReflect(no_diff float3 i, no_diff float3 n, no_diff out float3 r)
{
    r = reflect(i, n);
}

[Differentiable]
void callRefract(no_diff float3 i, no_diff float3 n, no_diff out float3 r)
{
    r = refract(i, n, 0.5);
}

[Differentiable]
void callDeterminant(no_diff float3x3 m, no_diff out float r)
{
    r = determinant(m);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float3 a;
    callReflect(float3(1, 0, 0), float3(0, 1, 0), a);
    float3 b;
    callRefract(float3(1, 0, 0), float3(0, 1, 0), b);
    float c;
    callDeterminant(float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1), c);
    outBuf[0] = a.x + b.x + c;
}
