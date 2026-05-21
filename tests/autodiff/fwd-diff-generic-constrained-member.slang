// Regression test for shader-slang/slang#11004:
// __fwd_diff applied to a member method on a generic-constrained
// interface-typed receiver previously ICEd in propagateConstExpr with
// "paramCount == callArgCount". The fix lowers the differentiated call
// so the operand list and callee `IRFuncType` agree on parameter
// counts even when the callee is an interface witness lookup whose
// signature includes an implicit `this`.
//
// The ICE only manifests through the front-end "non-essential
// validation" pass that runs propagateConstExpr; we exercise it via a
// SPIRV compile-only check so no GPU is required.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry fs -stage fragment

interface ICurve
{
    [Differentiable] float3 eval(float t);
}

struct Helix : ICurve
{
    [Differentiable] float3 eval(float t)
    {
        return float3(cos(t), 0.18 * t, sin(t));
    }
}

float3 tangent<C : ICurve>(C curve, float t)
{
    let pair = __fwd_diff(curve.eval)(diffPair(t, 1.0));
    return pair.d;
}

// CHECK: OpEntryPoint Fragment
[shader("fragment")]
float4 fs(float4 pos : SV_Position) : SV_Target
{
    Helix h;
    return float4(tangent(h, pos.x * 0.01), 1.0);
}
