//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test that integer matrices work on WGSL through lowering
    // WGSL natively only supports float matrices, so these should be lowered to arrays of vectors
    
    int2x2 mat1 = int2x2(1, 2, 3, 4);
    int3x3 mat2 = int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    
    // Test bool matrices too
    bool2x2 bmat = bool2x2(true, false, false, true);
    
    // Test basic operations that should work with lowered matrices
    int val1 = mat1[0][0];
    int val2 = mat1[1][1];
    int val3 = mat2[1][2];
    
    // Test row access
    int2 row = mat1[0];
    
    // Test element modification
    mat1[0][1] = 99;
    
    // Test bool matrix operations
    bool bval = bmat[0][0];
    
    // Store results
    outputBuffer[0] = val1; // 1
    outputBuffer[1] = val2; // 4
    outputBuffer[2] = val3; // 6
    outputBuffer[3] = row.x; // 1
    outputBuffer[4] = row.y; // 2
    outputBuffer[5] = mat1[0][1]; // 99
    outputBuffer[6] = bval ? 1 : 0; // 1
    outputBuffer[7] = mat2[2][2]; // 9
}

// CHECK: 1
// CHECK: 4
// CHECK: 6
// CHECK: 1
// CHECK: 2
// CHECK: 99
// CHECK: 1
// CHECK: 9 