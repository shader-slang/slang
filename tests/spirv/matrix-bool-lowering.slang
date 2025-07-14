// -O0 is to make sure that slang doesn't perform constant folding and make this test trivial
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -xslang -O0 -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[1 0], stride=4):in,name inputBuffer
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

// Global bool constants to avoid constant folding
static bool trueVal;
static bool falseVal;

struct matrixWrapper {
    bool2x2 mat1 = bool2x2(falseVal, falseVal, falseVal, falseVal);
    bool2x3 mat2 = bool2x3(trueVal, trueVal, falseVal, falseVal, falseVal, trueVal);
}

bool elementAnd(bool2x2 matrix)
{
    return trueVal
        && matrix[0][0]
        && matrix[0][1]
        && matrix[1][0]
        && matrix[1][1];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Load true/false values from input buffer to avoid constant folding
    trueVal = inputBuffer[0] != 0;
    falseVal = inputBuffer[1] != 0;
    
    // Test bool matrix construction
    bool2x2 mat1 = bool2x2(trueVal, falseVal, falseVal, trueVal);
    bool3x3 mat2 = bool3x3(
        trueVal, falseVal, trueVal,
        falseVal, trueVal, falseVal,
        trueVal, falseVal, trueVal
    );
    bool2x4 mat3 = bool2x4(
        trueVal, falseVal, trueVal, falseVal,
        trueVal, falseVal, trueVal, falseVal
    );
    
    // Test bool matrix element access
    bool val1 = mat1[0][0];
    bool val2 = mat2[2][1];
    
    // Test bool matrix row access
    bool2 row = mat1[1];
    bool3 row3 = mat2[0];
    
    // Test logical operations
    bool2x2 not_mat = !mat1;
    bool2x2 and_mat = mat1 && bool2x2(trueVal, trueVal, falseVal, falseVal);
    
    // Test element assignment
    mat1[0][1] = trueVal;
    mat2[1][2] = falseVal;

    // Test passing bool matrices to functions
    bool anded = elementAnd(mat1);

    // Test structs with bool matrix fields
    matrixWrapper wrapper = {};

    // Store results
    outputBuffer[0] = val1;
    outputBuffer[1] = val2;
    outputBuffer[2] = row.x;
    outputBuffer[3] = row.y;
    outputBuffer[4] = row3.y;
    outputBuffer[5] = not_mat[0][0];
    outputBuffer[6] = and_mat[0][0];
    outputBuffer[7] = mat1[0][1];
    outputBuffer[8] = mat3[0][1];
    outputBuffer[9] = anded;
    outputBuffer[10] = wrapper.mat1[0][0] || wrapper.mat2[0][0];
}

// CHECK: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1 
// CHECK-NEXT: 0 
// CHECK-NEXT: 0 
// CHECK-NEXT: 1
