//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// This test verifies that row_major and column_major matrices don't create
// duplicate DiffPair structs when used together in autodiff code.
// Before the fix, this would generate compilation errors due to mismatched
// DiffPair_matrixx3Cfloatx2C3x2C3x3E_0 and DiffPair_matrixx3Cfloatx2C3x2C3x3E_1 types.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[BackwardDifferentiable]
float3 matmul33_row(no_diff float3 v, row_major float3x3 w) {
    return mul(w, v);
}

[BackwardDifferentiable]
float3 matmul33_col(no_diff float3 v, float3x3 w) {
    return mul(w, v);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    // Test row_major matrix
    row_major float3x3 w_row = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    float3 v = float3(1.0, 1.0, 1.0);

    DifferentialPair<row_major float3x3> dpW_row = diffPair(w_row);
    __bwd_diff(matmul33_row)(v, dpW_row, float3(1.0, 1.0, 1.0));
    
    // Write gradients to output buffer to prevent dead code elimination
    outputBuffer[0] = dpW_row.d[0][0]; // CHECK: 1
    outputBuffer[1] = dpW_row.d[0][1]; // CHECK: 1  
    outputBuffer[2] = dpW_row.d[0][2]; // CHECK: 1
    
    // Test column_major matrix to ensure they share the same DiffPair struct
    float3x3 w_col = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    DifferentialPair<float3x3> dpW_col = diffPair(w_col);
    __bwd_diff(matmul33_col)(v, dpW_col, float3(1.0, 1.0, 1.0));
    
    outputBuffer[3] = dpW_col.d[0][0]; // CHECK: 1
    outputBuffer[4] = dpW_col.d[0][1]; // CHECK: 1
    outputBuffer[5] = dpW_col.d[0][2]; // CHECK: 1
    
    // Additional test values
    outputBuffer[6] = dpW_row.d[1][0]; // CHECK: 1
    outputBuffer[7] = dpW_col.d[1][0]; // CHECK: 1
    outputBuffer[8] = dpW_row.d[2][0]; // CHECK: 1
}