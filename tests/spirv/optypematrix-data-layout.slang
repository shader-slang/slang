// Test OpTypeMatrix emission with different data layouts (std140, std430, scalar).
// Uses both row_major and column_major on the same matrix type to show all layout differences.

// std140 data layout
//TEST:SIMPLE(filecheck=CHECK_STD140):-target spirv -entry computeMain -stage compute -DDATA_LAYOUT=Std140DataLayout
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_STD140):-vk -compute -output-using-type -xslang -DDATA_LAYOUT=Std140DataLayout

// std430 data layout
//TEST:SIMPLE(filecheck=CHECK_STD430):-target spirv -entry computeMain -stage compute -DDATA_LAYOUT=Std430DataLayout
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_STD430):-vk -compute -output-using-type -xslang -DDATA_LAYOUT=Std430DataLayout

// scalar data layout
//TEST:SIMPLE(filecheck=CHECK_SCALAR):-target spirv -entry computeMain -stage compute -DDATA_LAYOUT=ScalarDataLayout
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_SCALAR):-vk -compute -output-using-type -xslang -DDATA_LAYOUT=ScalarDataLayout


struct MatrixData
{
    row_major float3x2 m1;
    column_major float3x2 m2;
};

GLSLShaderStorageBuffer<MatrixData, DATA_LAYOUT> sbuf;
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: set sbuf = ubuffer(data=[0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0  0 0 0 0], stride=4)

// ====================================================================================================
// Memory layout with input data [0.0, 1.0, ..., 19.0]
//
// std140                              std430                              scalar
// --------------------------         ---------------------------          ---------------------------
//
// m1 (row_major float3x2):            m1 (row_major float3x2):            m1 (row_major float3x2):
//       col0 col1                           col0 col1                           col0 col1
// row0 [  0    1 ]  2    3            row0 [  0    1 ]                    row0 [  0    1 ]
// row1 [  4    5 ]  6    7            row1 [  2    3 ]                    row1 [  2    3 ]
// row2 [  8    9 ] 10   11            row2 [  4    5 ]                    row2 [  4    5 ]
//
//                                     (padding between m1 and m2)
//                                             6    7
//
// m2 (column_major float3x2):         m2 (column_major float3x2):         m2 (column_major float3x2):
//       col0 col1                           col0 col1                           col0 col1
// row0 [ 12   16 ]                    row0 [  8   12 ]                    row0 [  6    9 ]
// row1 [ 13   17 ]                    row1 [  9   13 ]                    row1 [  7   10 ]
// row2 [ 14   18 ]                    row2 [ 10   14 ]                    row2 [  8   11 ]
//        15   19                             11   15
//
// Offsets:                            Offsets:                            Offsets:
//   m1: 0, m2: 48                       m1: 0, m2: 32                       m1: 0, m2: 24
//   Total: 80 bytes                     Total: 64 bytes                     Total: 48 bytes
// ====================================================================================================

// Verify struct member decorations - std140
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 0 Offset 0
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 0 ColMajor
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 0 MatrixStride 16
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 1 Offset 48
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 1 RowMajor
//CHECK_STD140: OpMemberDecorate %MatrixData{{.*}} 1 MatrixStride 16


// Verify struct member decorations - std430
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 0 Offset 0
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 0 ColMajor
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 0 MatrixStride 8
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 1 Offset 32
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 1 RowMajor
//CHECK_STD430: OpMemberDecorate %MatrixData{{.*}} 1 MatrixStride 16


// Verify struct member decorations - scalar
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 0 Offset 0
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 0 ColMajor
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 0 MatrixStride 8
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 1 Offset 24
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 1 RowMajor
//CHECK_SCALAR: OpMemberDecorate %MatrixData{{.*}} 1 MatrixStride 12


// Verify OpTypeMatrix is used (not arrays of vectors)
// float3x2:
//CHECK_STD140: OpTypeMatrix %v2float 3
//CHECK_STD430: OpTypeMatrix %v2float 3
//CHECK_SCALAR: OpTypeMatrix %v2float 3


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // m1 (row_major float3x2) corners
    outputBuffer[0] = sbuf.m1[0][0];
    outputBuffer[1] = sbuf.m1[0][1];
    outputBuffer[2] = sbuf.m1[2][0];
    outputBuffer[3] = sbuf.m1[2][1];

    // m2 (column_major float3x2) corners
    outputBuffer[4] = sbuf.m2[0][0];
    outputBuffer[5] = sbuf.m2[0][1];
    outputBuffer[6] = sbuf.m2[2][0];
    outputBuffer[7] = sbuf.m2[2][1];
}


// Expected outputs for std140 data layout, checking all matrix corners
// m1:
//CHECK_COMPUTE_STD140: 0.0
//CHECK_COMPUTE_STD140: 1.0
//CHECK_COMPUTE_STD140: 8.0
//CHECK_COMPUTE_STD140: 9.0
// m2:
//CHECK_COMPUTE_STD140: 12.0
//CHECK_COMPUTE_STD140: 16.0
//CHECK_COMPUTE_STD140: 14.0
//CHECK_COMPUTE_STD140: 18.0

// Expected outputs for std430 data layout, checking all matrix corners
// m1:
//CHECK_COMPUTE_STD430: 0.0
//CHECK_COMPUTE_STD430: 1.0
//CHECK_COMPUTE_STD430: 4.0
//CHECK_COMPUTE_STD430: 5.0
// m2:
//CHECK_COMPUTE_STD430: 8.0
//CHECK_COMPUTE_STD430: 12.0
//CHECK_COMPUTE_STD430: 10.0
//CHECK_COMPUTE_STD430: 14.0

// Expected outputs for scalar data layout, checking all matrix corners
// m1:
//CHECK_COMPUTE_SCALAR: 0.0
//CHECK_COMPUTE_SCALAR: 1.0
//CHECK_COMPUTE_SCALAR: 4.0
//CHECK_COMPUTE_SCALAR: 5.0
// m2:
//CHECK_COMPUTE_SCALAR: 6.0
//CHECK_COMPUTE_SCALAR: 9.0
//CHECK_COMPUTE_SCALAR: 8.0
//CHECK_COMPUTE_SCALAR: 11.0
