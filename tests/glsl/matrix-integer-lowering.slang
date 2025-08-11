//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -vk -output-using-type -compute -emit-spirv-via-glsl -shaderobj -xslang -DTYPE=int
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -vk -output-using-type -compute -emit-spirv-via-glsl -shaderobj -xslang -DTYPE=uint

#ifndef TYPE
#define TYPE int
#endif

typealias m2x2 = matrix<TYPE, 2, 2>;
typealias m2x3 = matrix<TYPE, 2, 3>;
typealias m3x3 = matrix<TYPE, 3, 3>;
typealias m2x4 = matrix<TYPE, 2, 4>;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
//TEST_INPUT:ubuffer(data=[-1 4], stride=4):name expectedBuffer
RWStructuredBuffer<TYPE> outputBuffer;
RWStructuredBuffer<TYPE> expectedBuffer;

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
    
    // Test bit shift operations
    m2x2 shift_mat = m2x2(1, 2, 4, 8);
    m2x2 left_shift = shift_mat << 1;
    m2x2 right_shift = shift_mat >> 1;
    
    // Test comparison operations (these return bool matrices, cast to TYPE for output)
    m2x2 comp_mat1 = m2x2(1, 3, 2, 4);
    m2x2 comp_mat2 = m2x2(2, 2, 3, 3);
    
    matrix<bool, 2, 2> less_than = comp_mat1 < comp_mat2;
    matrix<bool, 2, 2> greater_than = comp_mat1 > comp_mat2;
    matrix<bool, 2, 2> less_equal = comp_mat1 <= comp_mat2;
    matrix<bool, 2, 2> greater_equal = comp_mat1 >= comp_mat2;
    matrix<bool, 2, 2> equal_to = comp_mat1 == comp_mat2;
    matrix<bool, 2, 2> not_equal = comp_mat1 != comp_mat2;
    
    // Test matrix negation operations
    m2x2 neg_mat = m2x2(1, -2, 3, -4);
    m2x2 negated = -neg_mat;
    
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
    outputBuffer[21] = trans2x2[0][0];
    // CHECK-NEXT: 1
    outputBuffer[22] = trans2x2[1][0];
    // CHECK-NEXT: 2
    outputBuffer[23] = trans2x3[0][0];
    // CHECK-NEXT: 5
    
    // Bit shift operation results
    outputBuffer[24] = left_shift[0][0];
    // CHECK-NEXT: 2
    outputBuffer[25] = left_shift[0][1];
    // CHECK-NEXT: 4
    outputBuffer[26] = right_shift[1][0];
    // CHECK-NEXT: 2
    outputBuffer[27] = right_shift[1][1];
    // CHECK-NEXT: 4
    
    // Comparison operation results (bool matrices cast to TYPE)
    outputBuffer[28] = TYPE(less_than[0][0]);
    // CHECK-NEXT: 1
    outputBuffer[29] = TYPE(less_than[0][1]);
    // CHECK-NEXT: 0
    outputBuffer[30] = TYPE(greater_than[0][1]);
    // CHECK-NEXT: 1
    outputBuffer[31] = TYPE(greater_than[1][1]);
    // CHECK-NEXT: 1
    outputBuffer[32] = TYPE(less_equal[0][0]);
    // CHECK-NEXT: 1
    outputBuffer[33] = TYPE(less_equal[0][1]);
    // CHECK-NEXT: 0
    outputBuffer[34] = TYPE(greater_equal[0][1]);
    // CHECK-NEXT: 1
    outputBuffer[35] = TYPE(greater_equal[1][0]);
    // CHECK-NEXT: 0
    outputBuffer[36] = TYPE(equal_to[0][0]);
    // CHECK-NEXT: 0
    outputBuffer[37] = TYPE(not_equal[0][0]);
    // CHECK-NEXT: 1
    outputBuffer[38] = TYPE(negated[0][0] == expectedBuffer[0]);
    // CHECK-NEXT: 1
    outputBuffer[39] = TYPE(negated[1][1] == expectedBuffer[1]);
    // CHECK-NEXT: 1
} 