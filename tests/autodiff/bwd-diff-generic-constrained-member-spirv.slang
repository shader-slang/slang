// SPIRV-target lock-down for the bwd-mode side of shader-slang/slang#11004.
//
// `BwdDiffFuncType::_resolveImplOverride` produces a different signature
// shape than `FwdDiffFuncType` (`out` parameter for the gradient, distinct
// intermediate-context plumbing) and the resulting IR call sites flow
// through different propagate / primal-split passes before reaching SPIRV
// emission. The CPU-compute bwd test covers the IR-lowering fix; this
// file pins the SPIRV emit path so a SPIRV-only regression on the bwd
// side can't slip through.
//
// `ScalarCurve.eval(t) = k * t * t`, `∂eval/∂t = 2*k*t`. Anchor literals
// (3.14159, 2.71828) are non-trivial so a wrong-receiver miscompile that
// silently elided `s.k` would also elide the constant from the SPIRV
// output and fail the CHECK loudly.
//
// Pre-fix failure mode (verified against prebuilt slangc 2026.9.1):
//   bwd-mode lowering segfaults during compilation.

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
