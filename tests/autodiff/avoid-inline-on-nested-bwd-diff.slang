//TEST:SIMPLE(filecheck=CHECK): -target cuda

// Test for issue #8777: crash when combining [ForceInline], [Differentiable], and nested bwd_diff
// The test should compile without crashing

[ForceInline]
[Differentiable]
float2 fun0(float3 u0, no_diff float4 u1) {
    return float2(length(u0), length(u1));
}

[ForceInline]
[Differentiable]
float2x3 fun0_jac(float3 u0, no_diff float4 u1) {
    float2x3 J;
    for (int i = 0; i < 2; ++i) {
        DifferentialPair<float3> dp_u0 = diffPair(u0);
        bwd_diff(fun0)(dp_u0, u1, float2(1-i, i));
        J[i] = dp_u0.d;
    }
    return J;
}

[CudaDeviceExport]
[Differentiable]
float2x2 fun1(const float3 u0, no_diff const float4 u1) {
    float2x3 J = fun0_jac(u0, u1);
    return mul(J, transpose(J));
}

[CudaDeviceExport]
float3 fun1_vjp(const float3 u0, no_diff const float4 u1, const float2x2 g_v) {
    DifferentialPair<float3> dp_u0 = diffPair(u0);
    bwd_diff(fun1)(dp_u0, u1, g_v);
    return dp_u0.d;
}

// CHECK: fun1
// CHECK: fun1_vjp
