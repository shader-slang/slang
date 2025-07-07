//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test that bool matrices work on GLSL through lowering
    // GLSL has no native bool matrix types, so these must be lowered to arrays of bool vectors
    
    bool2x2 bmat1 = bool2x2(true, false, false, true);
    bool3x3 bmat2 = bool3x3(
        true, false, true,
        false, true, false,
        true, false, true
    );
    
    // Test bool matrix element access
    bool b_val1 = bmat1[0][0];
    bool b_val2 = bmat2[2][1];
    
    // Test bool matrix row access
    bool2 brow = bmat1[1];
    bool3 brow3 = bmat2[0];
    
    // Test logical operations
    bool2x2 not_bmat = !bmat1;
    bool2x2 and_bmat = bmat1 && bool2x2(true, true, false, false);
    
    // Test element assignment
    bmat1[0][1] = true;
    bmat2[1][2] = false;
    
    // Store results
    outputBuffer[0] = b_val1 ? 1 : 0; // true = 1
    outputBuffer[1] = b_val2 ? 1 : 0; // false = 0
    outputBuffer[2] = brow.x ? 1 : 0; // false = 0
    outputBuffer[3] = brow.y ? 1 : 0; // true = 1
    outputBuffer[4] = brow3.y ? 1 : 0; // false = 0
    outputBuffer[5] = not_bmat[0][0] ? 1 : 0; // !true = false = 0
    outputBuffer[6] = and_bmat[0][0] ? 1 : 0; // true && true = 1
    outputBuffer[7] = bmat1[0][1] ? 1 : 0; // modified to true = 1
}

// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 1
// CHECK: 1 