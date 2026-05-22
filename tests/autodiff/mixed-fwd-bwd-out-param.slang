//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// Regression test for shader-slang/slang#11160.
// A bwd_diff'd function whose body invokes fwd_diff on a callee that has
// `out`-differentiable parameters used to crash the autodiff transpose pass
// with "Unrecognized field. Cannot emit field accessor" because the rev-mode
// gradient threading produced IRDifferentialPairGetPrimal/Differential and
// IRMakeDifferentialPair instructions on bare differential values rather than
// on pair values.

[Differentiable]
void proj(float3 xyz, out float2 uv)
{
    uv = float2(xyz.x / xyz.z, xyz.y / xyz.z);
}

[Differentiable]
float2x3 proj_jac(float3 p_view)
{
    float2x3 J;
    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
    {
        DifferentialPair<float3> dp_p_view =
            diffPair(p_view, float3(float(i == 0), float(i == 1), float(i == 2)));
        DifferentialPair<float2> dp_uv = diffPair(float2(0, 0));
        fwd_diff(proj)(dp_p_view, dp_uv);
        J[0][i] = dp_uv.d.x;
        J[1][i] = dp_uv.d.y;
    }
    return J;
}

[CudaDeviceExport]
[Differentiable]
void proj_with_cov(
    const float3 xyz,
    const float3x3 xyz_cov,
    out float2 uv,
    out float2x2 uv_cov)
{
    proj(xyz, uv);
    float2x3 J = proj_jac(xyz);
    uv_cov = mul(mul(J, xyz_cov), transpose(J));
}

//CUDA: proj_with_cov_vjp
[CudaDeviceExport]
void proj_with_cov_vjp(
    const float3 xyz,
    const float3x3 xyz_cov,
    const float2 v_uv,
    const float2x2 v_uv_cov,
    out float3 v_xyz,
    out float3x3 v_xyz_cov)
{
    DifferentialPair<float3> dp_xyz = diffPair(xyz);
    DifferentialPair<float3x3> dp_xyz_cov = diffPair(xyz_cov);
    bwd_diff(proj_with_cov)(dp_xyz, dp_xyz_cov, v_uv, v_uv_cov);
    v_xyz = dp_xyz.d;
    v_xyz_cov = dp_xyz_cov.d;
}
