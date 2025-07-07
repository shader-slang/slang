//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test that integer matrices work on SPIRV through lowering
    // SPIRV has Matrix capability but may not support integer matrices universally
    
    int2x2 mat1 = int2x2(10, 20, 30, 40);
    uint2x2 umat = uint2x2(100u, 200u, 300u, 400u);
    bool2x2 bmat = bool2x2(true, false, true, false);
    
    // Test matrix construction and access
    int2x2 mat2 = int2x2(int2(1, 2), int2(3, 4));
    
    // Test basic operations
    int val1 = mat1[0][0];
    uint val2 = umat[1][1];
    bool val3 = bmat[0][1];
    
    // Test matrix operations
    int2x2 sum = mat1 + mat2;
    bool2x2 cmp = (mat1 == mat2);
    
    // Test matrix element assignment
    mat1[1][0] = 99;
    
    // Store results
    outputBuffer[0] = val1; // 10
    outputBuffer[1] = int(val2); // 400
    outputBuffer[2] = val3 ? 1 : 0; // 0
    outputBuffer[3] = sum[0][0]; // 10 + 1 = 11
    outputBuffer[4] = sum[1][1]; // 40 + 4 = 44
    outputBuffer[5] = cmp[0][0] ? 1 : 0; // false = 0
    outputBuffer[6] = mat1[1][0]; // 99
    outputBuffer[7] = mat2[1][1]; // 4
}

// CHECK: 10
// CHECK: 400
// CHECK: 0
// CHECK: 11
// CHECK: 44
// CHECK: 0
// CHECK: 99
// CHECK: 4 