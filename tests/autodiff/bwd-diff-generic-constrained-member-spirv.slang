// `__bwd_diff(curve.eval)` for a generic-constrained receiver, compiled to SPIR-V.
// `ScalarCurve.eval(t) = k * t * t`; the check is that the entry point compiles and that the
// receiver's `k` (3.14159) stays live.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry fs

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

struct ScalarCurve : ICurve
{
    float k;

    [Differentiable]
    float eval(float t)
    {
        return k * t * t;
    }
}

float gradient<C : ICurve>(C curve, float t)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, 1.0);
    return pair.d;
}

// CHECK-DAG: OpEntryPoint Fragment
// CHECK-DAG: OpConstant %float 3.14159
// CHECK-DAG: OpConstant %float 2.71828
[shader("fragment")]
float4 fs(float4 pos : SV_Position) : SV_Target
{
    ScalarCurve s;
    s.k = 3.14159;
    return float4(gradient(s, pos.x * 2.71828), 0, 0, 1);
}
