//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -output-using-type -compute -vk -shaderobj -xslang -emit-spirv-directly -xslang -DTYPE=int
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -output-using-type -compute -vk -shaderobj -xslang -emit-spirv-directly -xslang -DTYPE=uint

#ifndef TYPE
#define TYPE int
#endif

typealias m2x2 = matrix<TYPE, 2, 2>;
typealias m2x3 = matrix<TYPE, 2, 3>;
typealias m3x3 = matrix<TYPE, 3, 3>;
typealias m2x4 = matrix<TYPE, 2, 4>;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<TYPE> outputBuffer;

struct matrixWrapper {
    m2x2 mat1 = m2x2(1, 2, 3, 4);
    m2x3 mat2 = m2x3(5, 6, 7, 8, 9, 10);
};

TYPE elementAdd(m2x2 matrix)
{
    return matrix[0][0]
        + matrix[0][1]
        + matrix[1][0]
        + matrix[1][1];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test matrix construction
    m2x2 mat1 = m2x2(1, 2, 3, 4);
    m3x3 mat2 = m3x3(
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    );
    m2x4 mat3 = m2x4(
        10, 11, 12, 13,
        14, 15, 16, 17
    );
    
    // Test matrix element access
    TYPE val1 = mat1[0][0];
    TYPE val2 = mat2[2][1];
    
    // Test matrix row access
    vector<TYPE, 2> row = mat1[1];
    vector<TYPE, 3> row3 = mat2[0];
    
    // Test arithmetic operations
    m2x2 mat5 = m2x2(2, 4, 6, 7);
    
    m2x2 mat_scalar = 2 * mat1;
    m2x2 mat_add = mat1 + mat5;
    m2x2 mat_sub = mat5 - mat1;
    m2x2 mat_mul = mat1 * mat5;
    
    // Test passing matrices to functions
    TYPE added = elementAdd(mat1);

    // Test structs with matrix fields
    matrixWrapper wrapper = {};
    
    // Test matrix intrinsic operations

    // Test determinant for square matrices
    m2x2 mat6 = m2x2(2, 1, 4, 3);
    TYPE det2x2 = TYPE(determinant(mat6));
    TYPE det3x3 = TYPE(determinant(mat2));
    
    // Test transpose
    matrix<TYPE, 2, 2> trans2x2 = transpose(mat1);
    matrix<TYPE, 3, 2> trans2x3 = transpose(wrapper.mat2);
    
    // Test element-wise min/max
    m2x2 mat_min = min(mat1, mat5);
    m2x2 mat_max = max(mat1, mat5);
    
    // Test all/any operations (these return bool, but we'll cast to TYPE for output)
    m2x2 zero_mat = m2x2(0, 0, 0, 0);
    m2x2 mixed_mat = m2x2(1, 0, 2, 0);
    
    TYPE all_nonzero = TYPE(all(mat1));
    TYPE all_zero = TYPE(all(zero_mat));
    TYPE any_nonzero = TYPE(any(mixed_mat));
    TYPE any_zero = TYPE(any(zero_mat));
    
    // Store results
    outputBuffer[0] = val1;
    // CHECK: 1
    outputBuffer[1] = val2;
    // CHECK-NEXT: 8
    outputBuffer[2] = row.x;
    // CHECK-NEXT: 3
    outputBuffer[3] = row.y;
    // CHECK-NEXT: 4
    outputBuffer[4] = row3.y;
    // CHECK-NEXT: 2
    outputBuffer[5] = mat_scalar[0][0];
    // CHECK-NEXT: 2
    outputBuffer[6] = mat_add[0][0];
    // CHECK-NEXT: 3
    outputBuffer[7] = mat_sub[0][0];
    // CHECK-NEXT: 1
    outputBuffer[8] = mat_mul[1][1];
    // CHECK-NEXT: 28
    outputBuffer[9] = added;
    // CHECK-NEXT: 10
    outputBuffer[10] = wrapper.mat1[0][0] * wrapper.mat2[0][0];
    // CHECK-NEXT: 5
    
    // Matrix intrinsic operation results
    outputBuffer[11] = det2x2;
    // CHECK-NEXT: 2
    outputBuffer[12] = det3x3;
    // CHECK-NEXT: 0
    outputBuffer[13] = mat_min[0][0];
    // CHECK-NEXT: 1
    outputBuffer[14] = mat_min[1][1];
    // CHECK-NEXT: 4
    outputBuffer[15] = mat_max[0][0];
    // CHECK-NEXT: 2
    outputBuffer[16] = mat_max[1][1];
    // CHECK-NEXT: 7
    outputBuffer[17] = all_nonzero;
    // CHECK-NEXT: 1
    outputBuffer[18] = all_zero;
    // CHECK-NEXT: 0
    outputBuffer[19] = any_nonzero;
    // CHECK-NEXT: 1
    outputBuffer[20] = any_zero;
    // CHECK-NEXT: 0
    outputBuffer[21] = trans2x2[0][0];  // transpose([[1,2],[3,4]])[0][0] = 1
    // CHECK-NEXT: 1
    outputBuffer[22] = trans2x2[1][0];  // transpose([[1,2],[3,4]])[1][0] = 2 
    // CHECK-NEXT: 2
    outputBuffer[23] = trans2x3[0][0];  // transpose(wrapper.mat2)[0][0] = 5
    // CHECK-NEXT: 5
}