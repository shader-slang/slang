//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -output-using-type -compute -vk -shaderobj -xslang -O0 -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct matrixWrapper {
    int2x2 mat1 = int2x2(1, 2, 3, 4);
    int2x3 mat2 = int2x3(5, 6, 7, 8, 9, 10);
}

int elementAdd(int2x2 matrix)
{
    return matrix[0][0]
        + matrix[0][1]
        + matrix[1][0]
        + matrix[1][1];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test int matrix construction
    int2x2 mat1 = int2x2(1, 2, 3, 4);
    int3x3 mat2 = int3x3(
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    );
    int2x4 mat3 = int2x4(
        10, 11, 12, 13,
        14, 15, 16, 17
    );
    
    // Test int matrix element access
    int val1 = mat1[0][0];
    int val2 = mat2[2][1];
    
    // Test bool matrix row access
    int2 row = mat1[1];
    int3 row3 = mat2[0];
    
    // Test arithmetic operations
    int2x2 mat5 = int2x2(2, 4, 6, 7);
    
    int2x2 mat_scalar = 2 * mat1;
    int2x2 mat_add = mat1 + mat5;
    int2x2 mat_sub = mat5 - mat1;
    int2x2 mat_mul = mat1 * mat5;
    
    // Test passing bool matrices to functions
    int added = elementAdd(mat1);

    // Test structs with bool matrix fields
    matrixWrapper wrapper = {};
    
    // Store results
    outputBuffer[0] = val1;
    outputBuffer[1] = val2;
    outputBuffer[2] = row.x;
    outputBuffer[3] = row.y;
    outputBuffer[4] = row3.y;
    outputBuffer[5] = mat_scalar[0][0];
    outputBuffer[6] = mat_add[0][0];
    outputBuffer[7] = mat_sub[0][0];
    outputBuffer[8] = mat_mul[1][1];
    outputBuffer[9] = added;
    outputBuffer[10] = wrapper.mat1[0][0] * wrapper.mat2[0][0];
}

// CHECK: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: 28
// CHECK-NEXT: 10
// CHECK-NEXT: 5