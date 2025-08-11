//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -d3d11 -output-using-type

// This test verifies that row_major and column_major matrices don't create
// duplicate DiffPair structs when used together in autodiff code.
// Before the fix, this would generate compilation errors due to mismatched
// DiffPair_matrixx3Cfloatx2C3x2C3x3E_0 and DiffPair_matrixx3Cfloatx2C3x2C3x3E_1 types.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float3 matmul33_row(no_diff float3 v, row_major float3x3 w) {
    return mul(w, v);
}

[Differentiable]
float3 matmul33_col(no_diff float3 v, column_major float3x3 w) {
    return mul(w, v);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    // Test row_major matrix with meaningful values
    row_major float3x3 w_row = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    float3 v = float3(1.0, 2.0, 3.0);

    DifferentialPair<row_major float3x3> dpW_row = diffPair(w_row);
    __bwd_diff(matmul33_row)(v, dpW_row, float3(4.0, 5.0, 6.0));
    
    // Write gradients to output buffer to prevent dead code elimination
    // Expected gradient matrix is dResult ⊗ v = [4,5,6]^T ⊗ [1,2,3] = [[4,8,12],[5,10,15],[6,12,18]]
    outputBuffer[0] = dpW_row.d[0][0]; // CHECK: 4
    outputBuffer[1] = dpW_row.d[0][1]; // CHECK: 8  
    outputBuffer[2] = dpW_row.d[0][2]; // CHECK: 12

    // Test column_major matrix to ensure they share the same DiffPair struct
    column_major float3x3 w_col = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    DifferentialPair<column_major float3x3> dpW_col = diffPair(w_col);
    __bwd_diff(matmul33_col)(v, dpW_col, float3(4.0, 5.0, 6.0));
    
    outputBuffer[3] = dpW_col.d[1][0]; // CHECK: 5
    outputBuffer[4] = dpW_col.d[1][1]; // CHECK: 10
    outputBuffer[5] = dpW_col.d[1][2]; // CHECK: 15
    
    // Additional test values from different matrix positions
    outputBuffer[6] = dpW_row.d[2][0]; // CHECK: 6
    outputBuffer[7] = dpW_col.d[2][1]; // CHECK: 12
    outputBuffer[8] = dpW_row.d[2][2]; // CHECK: 18
}
