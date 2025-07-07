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
    // Test basic integer matrix types
    int2x2 mat2x2 = int2x2(1, 2, 3, 4);
    int3x3 mat3x3 = int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    int4x4 mat4x4 = int4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    
    // Test unsigned integer matrices
    uint2x2 umat2x2 = uint2x2(1u, 2u, 3u, 4u);
    uint3x3 umat3x3 = uint3x3(1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u);
    
    // Test element access
    int val1 = mat2x2[0][0];  // Should be 1
    int val2 = mat3x3[1][2];  // Should be 6
    uint val3 = umat2x2[1][0]; // Should be 3
    
    // Test row access
    int2 row0 = mat2x2[0];    // Should be (1, 2)
    int3 row1 = mat3x3[1];    // Should be (4, 5, 6)
    
    // Test matrix construction from rows
    int2x2 mat2x2_from_rows = int2x2(int2(10, 20), int2(30, 40));
    
    // Test matrix modification
    mat2x2[0][1] = 99;        // Modify element
    mat3x3[2] = int3(77, 88, 99); // Modify row
    
    // Test matrix initialization with scalar
    int2x2 scalar_mat = int2x2(42);
    
    // Test matrix addition
    int2x2 sum_mat = mat2x2 + mat2x2_from_rows;
    
    // Store results for verification
    outputBuffer[0] = val1;
    outputBuffer[1] = val2;
    outputBuffer[2] = int(val3);
    outputBuffer[3] = row0.x;
    outputBuffer[4] = row0.y;
    outputBuffer[5] = row1.x;
    outputBuffer[6] = row1.y;
    outputBuffer[7] = row1.z;
    outputBuffer[8] = mat2x2[0][1]; // Should be 99
    outputBuffer[9] = mat3x3[2][0]; // Should be 77
    outputBuffer[10] = scalar_mat[0][0]; // Should be 42
    outputBuffer[11] = scalar_mat[1][1]; // Should be 42
    outputBuffer[12] = sum_mat[0][0]; // Should be 11 (1+10)
    outputBuffer[13] = sum_mat[1][1]; // Should be 44 (4+40)
    outputBuffer[14] = mat4x4[3][3]; // Should be 16
    outputBuffer[15] = umat3x3[2][2]; // Should be 9
}

// CHECK: 1
// CHECK: 6
// CHECK: 3
// CHECK: 1
// CHECK: 2
// CHECK: 4
// CHECK: 5
// CHECK: 6
// CHECK: 99
// CHECK: 77
// CHECK: 42
// CHECK: 42
// CHECK: 11
// CHECK: 44
// CHECK: 16
// CHECK: 9 