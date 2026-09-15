// SPIRV-target lock-down for shader-slang/slang#11004.
//
// The companion file `fwd-diff-generic-constrained-member.slang` covers
// the IR-lowering fix on the CPU compute path. This test mirrors the
// shape of the issue-body reproducer — a fragment shader that invokes
// `__fwd_diff(curve.eval)(diffPair(t, 1.0))` for `curve : C` with
// `C : ICurve` — and locks it to the SPIRV emitter so a future
// SPIRV-only regression in fragment-stage emission can't silently
// re-break the user-visible shape #11004 reports.
//
// The struct used here is `ScalarCurve` (`eval(t) = k * t * t`,
// derivative `2 * k * t`) rather than the literal issue-body `Helix`
// because `Helix` has no fields, so a wrong-receiver miscompile that
// silently dropped the implicit `this` would still emit valid SPIRV
// indistinguishable from a correct emit. With `ScalarCurve.k` flowing
// into the derivative, the lowered SPIRV must reference `k`'s
// initialiser.
//
// Anchor literals are deliberately non-trivial (3.14159, 2.71828) to
// prevent constant-folding from collapsing a `3.0` into a different
// precomputed value, or another stray `3.0` (default initialiser,
// layout constant) from satisfying the CHECK incidentally. If the
// receiver were dropped, `k` would not be live and `OpConstant %float
// 3.14159` would be elided.
//
// Pre-fix failure mode (verified against prebuilt slangc 2026.9.1):
//   error[E99997]: ... assert failure: slang-ir-constexpr.cpp(504):
//   paramCount == callArgCount

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

float tangent<C : ICurve>(C curve, float t)
{
    let pair = __fwd_diff(curve.eval)(diffPair(t, 1.0));
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
    return float4(tangent(s, pos.x * 2.71828), 0, 0, 1);
}
