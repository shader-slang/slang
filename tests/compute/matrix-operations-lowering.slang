//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -use-dxil -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test matrix construction with different approaches
    int2x2 mat1 = int2x2(1, 2, 3, 4);
    int2x2 mat2 = {5, 6, 7, 8};
    int2x2 mat3 = int2x2(int2(9, 10), int2(11, 12));
    
    // Test matrix casting between types
    float2x2 fmat = float2x2(1.5f, 2.5f, 3.5f, 4.5f);
    int2x2 imat = int2x2(fmat);  // Cast float to int
    uint2x2 umat = uint2x2(mat1); // Cast int to uint
    
    // Test mixed-type matrix operations
    int2x2 sum1 = mat1 + mat2;
    int2x2 sub1 = mat2 - mat1;
    int2x2 mul1 = mat1 * 2;  // Scalar multiplication
    
    // Test matrix comparison
    bool2x2 eq_result = (mat1 == mat2);
    bool2x2 ne_result = (mat1 != mat2);
    
    // Test matrix transpose (if supported)
    int2x2 trans = transpose(mat1);
    
    // Test matrix-vector multiplication
    int2 vec = int2(1, 2);
    int2 mvec_result = mul(vec, mat1);
    
    // Test matrix-matrix multiplication
    int2x2 mm_result = mul(mat1, mat2);
    
    // Test matrix assignment and modification
    int2x2 modify_mat = mat1;
    modify_mat[0] = int2(99, 100);
    modify_mat[1][1] = 101;
    
    // Test matrix swizzle operations
    int4 mat_as_vec = int4(mat1[0], mat1[1]);
    int2x2 vec_as_mat = int2x2(mat_as_vec.xy, mat_as_vec.zw);
    
    // Test zero and identity matrix construction
    int2x2 zero_mat = int2x2(0);
    int2x2 identity_mat = int2x2(1, 0, 0, 1);
    
    // Store results for verification
    outputBuffer[0] = mat1[0][0] + mat2[0][0]; // 1 + 5 = 6
    outputBuffer[1] = imat[0][0]; // Should be 1 (cast from 1.5f)
    outputBuffer[2] = int(umat[1][1]); // Should be 4 (cast from mat1)
    outputBuffer[3] = sum1[0][0]; // 1 + 5 = 6
    outputBuffer[4] = sub1[1][1]; // 8 - 4 = 4
    outputBuffer[5] = mul1[0][1]; // 2 * 2 = 4
    outputBuffer[6] = eq_result[0][0] ? 1 : 0; // false = 0
    outputBuffer[7] = ne_result[0][0] ? 1 : 0; // true = 1
    outputBuffer[8] = trans[1][0]; // transpose: should be 2
    outputBuffer[9] = mvec_result[0]; // matrix-vector mult result
    outputBuffer[10] = mm_result[0][0]; // matrix-matrix mult result
    outputBuffer[11] = modify_mat[0][0]; // Should be 99
    outputBuffer[12] = modify_mat[1][1]; // Should be 101
    outputBuffer[13] = vec_as_mat[0][0]; // Should be same as mat1[0][0] = 1
    outputBuffer[14] = zero_mat[0][0]; // Should be 0
    outputBuffer[15] = identity_mat[1][0]; // Should be 0
}

// CHECK: 6
// CHECK: 1
// CHECK: 4
// CHECK: 6
// CHECK: 4
// CHECK: 4
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 7
// CHECK: 19
// CHECK: 99
// CHECK: 101
// CHECK: 1
// CHECK: 0
// CHECK: 0 