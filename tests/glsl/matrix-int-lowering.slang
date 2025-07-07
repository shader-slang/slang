//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test that integer matrices work on GLSL through lowering
    // GLSL has matrix types but they are traditionally float-only
    
    int2x2 imat = int2x2(1, 2, 3, 4);
    uint3x3 umat = uint3x3(1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u);
    bool2x2 bmat = bool2x2(true, false, false, true);
    
    // Test matrix indexing
    int i_val = imat[0][0];
    uint u_val = umat[2][1];
    bool b_val = bmat[1][1];
    
    // Test row access
    int2 irow = imat[1];
    uint3 urow = umat[0];
    
    // Test matrix assignment
    imat[0][1] = 99;
    umat[1][2] = 888u;
    
    // Store results
    outputBuffer[0] = i_val; // 1
    outputBuffer[1] = int(u_val); // 8
    outputBuffer[2] = b_val ? 1 : 0; // 1
    outputBuffer[3] = irow.x; // 3
    outputBuffer[4] = irow.y; // 4
    outputBuffer[5] = int(urow.x); // 1
    outputBuffer[6] = imat[0][1]; // 99
    outputBuffer[7] = int(umat[1][2]); // 888
}

// CHECK: 1
// CHECK: 8
// CHECK: 1
// CHECK: 3
// CHECK: 4
// CHECK: 1
// CHECK: 99
// CHECK: 888 