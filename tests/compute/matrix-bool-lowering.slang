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
    // Test basic bool matrix types
    bool2x2 mat2x2 = bool2x2(true, false, false, true);
    bool3x3 mat3x3 = bool3x3(true, false, true, false, true, false, true, false, true);
    bool4x4 mat4x4 = bool4x4(
        true, false, true, false,
        false, true, false, true,
        true, false, true, false,
        false, true, false, true
    );
    
    // Test element access
    bool val1 = mat2x2[0][0];  // Should be true
    bool val2 = mat3x3[1][2];  // Should be false
    bool val3 = mat4x4[2][1];  // Should be false
    
    // Test row access
    bool2 row0 = mat2x2[0];    // Should be (true, false)
    bool3 row1 = mat3x3[1];    // Should be (false, true, false)
    
    // Test matrix construction from rows
    bool2x2 mat2x2_from_rows = bool2x2(bool2(false, true), bool2(true, false));
    
    // Test matrix modification
    mat2x2[0][1] = true;       // Modify element
    mat3x3[2] = bool3(false, false, false); // Modify row
    
    // Test matrix initialization with scalar
    bool2x2 scalar_mat = bool2x2(true);
    
    // Test logical operations
    bool2x2 and_mat = mat2x2 && mat2x2_from_rows;
    bool2x2 or_mat = mat2x2 || mat2x2_from_rows;
    bool2x2 not_mat = !mat2x2;
    
    // Test comparison operations
    bool2x2 comp_mat = bool2x2(true, true, false, false);
    bool2x2 eq_mat = (mat2x2_from_rows == comp_mat);
    bool2x2 ne_mat = (mat2x2_from_rows != comp_mat);
    
    // Store results for verification (converting bool to int for output)
    outputBuffer[0] = val1 ? 1 : 0;
    outputBuffer[1] = val2 ? 1 : 0;
    outputBuffer[2] = val3 ? 1 : 0;
    outputBuffer[3] = row0.x ? 1 : 0;
    outputBuffer[4] = row0.y ? 1 : 0;
    outputBuffer[5] = row1.x ? 1 : 0;
    outputBuffer[6] = row1.y ? 1 : 0;
    outputBuffer[7] = row1.z ? 1 : 0;
    outputBuffer[8] = mat2x2[0][1] ? 1 : 0; // Should be true (1)
    outputBuffer[9] = mat3x3[2][0] ? 1 : 0; // Should be false (0)
    outputBuffer[10] = scalar_mat[0][0] ? 1 : 0; // Should be true (1)
    outputBuffer[11] = scalar_mat[1][1] ? 1 : 0; // Should be true (1)
    outputBuffer[12] = and_mat[0][0] ? 1 : 0; // Should be false (0)
    outputBuffer[13] = or_mat[1][1] ? 1 : 0; // Should be true (1)
    outputBuffer[14] = not_mat[0][0] ? 1 : 0; // Should be false (0)
    outputBuffer[15] = eq_mat[0][0] ? 1 : 0; // Should be false (0)
}

// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 0 