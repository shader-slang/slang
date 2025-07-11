// This test verifies that row_major and column_major matrices don't create
// duplicate DiffPair structs when used together in autodiff code.
// Before the fix, this would generate compilation errors due to mismatched
// DiffPair_matrixx3Cfloatx2C3x2C3x3E_0 and DiffPair_matrixx3Cfloatx2C3x2C3x3E_1 types.

[Differentiable]
float3 matmul33(no_diff float3 v, row_major float3x3 w) {
    return mul(w, v);
}

[Shader("compute")]
[NumThreads(1, 1, 1)]
void computeMain(int3 dispathThreadID: SV_DispatchThreadID) {
    row_major float3x3 w = float3x3(1.5,2.0,3.1, 5.2,8.8,9.4, 2.6,8.3,2.7);
    float3 v = float3(0.5, 0.2, 0.3);

    DifferentialPair<row_major float3x3> dpW = diffPair(w);
    bwd_diff(matmul33)(v, dpW, float3(1));

    printf("R: %f %f %f %f %f %f %f %f %f\n", 
           dpW.d[0][0], dpW.d[0][1], dpW.d[0][2],
           dpW.d[1][0], dpW.d[1][1], dpW.d[1][2],
           dpW.d[2][0], dpW.d[2][1], dpW.d[2][2]);
}