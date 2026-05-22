// SPIRV-target lock-down for shader-slang/slang#11004.
//
// The companion file `fwd-diff-generic-constrained-member.slang` covers
// the IR-lowering fix on the CPU compute path. This test mirrors the
// exact reproducer shape from the issue body — a fragment shader that
// invokes `__fwd_diff(curve.eval)(diffPair(t, 1.0))` for `curve : C`
// with `C : ICurve` — and locks it to the SPIRV emitter so a future
// SPIRV-only regression in fragment-stage emission can't silently
// re-break the user-visible shape #11004 reports.
//
// Pre-fix failure mode (verified against prebuilt slangc 2026.9.1):
//   error[E99997]: ... assert failure: slang-ir-constexpr.cpp(504):
//   paramCount == callArgCount

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry fs

interface ICurve
{
    [Differentiable]
    float3 eval(float t);
}

struct Helix : ICurve
{
    [Differentiable]
    float3 eval(float t)
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
// CHECK: OpFunction
[shader("fragment")]
float4 fs(float4 pos : SV_Position) : SV_Target
{
    Helix h;
    return float4(tangent(h, pos.x * 0.01), 1.0);
}
