// Test matrix layout modifiers (column_major, row_major) on struct members in
// uniform and storage buffers. Verifies modifiers override compilation options
// and that matrices are emitted as OpTypeMatrix (not lowered to arrays of vectors).

// No matrix layout modifier with -matrix-layout-column-major option
//TEST:SIMPLE(filecheck=CHECK_COLUMN):-target spirv -entry computeMain -stage compute -matrix-layout-column-major -DMATRIX_LAYOUT
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_COLUMN):-vk -compute -output-using-type -xslang -matrix-layout-column-major -xslang -DMATRIX_LAYOUT

// No matrix layout modifier with -matrix-layout-row-major option
//TEST:SIMPLE(filecheck=CHECK_ROW):-target spirv -entry computeMain -stage compute -matrix-layout-row-major -DMATRIX_LAYOUT
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_ROW):-vk -compute -output-using-type -xslang -matrix-layout-row-major -xslang -DMATRIX_LAYOUT

// column_major matrix layout modifier with -matrix-column-major option
//TEST:SIMPLE(filecheck=CHECK_COLUMN):-target spirv -entry computeMain -stage compute -matrix-layout-column-major -DMATRIX_LAYOUT=column_major
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_COLUMN):-vk -compute -output-using-type -xslang -matrix-layout-column-major -xslang -DMATRIX_LAYOUT=column_major

// column_major matrix layout modifier with -matrix-row-major option (expect column major)
//TEST:SIMPLE(filecheck=CHECK_COLUMN):-target spirv -entry computeMain -stage compute -matrix-layout-row-major -DMATRIX_LAYOUT=column_major
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_COLUMN):-vk -compute -output-using-type -xslang -matrix-layout-row-major -xslang -DMATRIX_LAYOUT=column_major

// row_major matrix layout modifier with -matrix-column-major option (expect row major)
//TEST:SIMPLE(filecheck=CHECK_ROW):-target spirv -entry computeMain -stage compute -matrix-layout-column-major -DMATRIX_LAYOUT=row_major
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_ROW):-vk -compute -output-using-type -xslang -matrix-layout-column-major -xslang -DMATRIX_LAYOUT=row_major

// row_major matrix layout modifier with -matrix-row-major option
//TEST:SIMPLE(filecheck=CHECK_ROW):-target spirv -entry computeMain -stage compute -matrix-layout-row-major -DMATRIX_LAYOUT=row_major
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_COMPUTE_ROW):-vk -compute -output-using-type -xslang -matrix-layout-row-major -xslang -DMATRIX_LAYOUT=row_major


struct MatrixData
{
    MATRIX_LAYOUT float4x4 m1;
    MATRIX_LAYOUT float2x4 m2;
    MATRIX_LAYOUT float3x2 m3;
};

ConstantBuffer<MatrixData, Std140DataLayout> cbuf;
GLSLShaderStorageBuffer<MatrixData, Std140DataLayout> sbuf;
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: set cbuf = cbuffer(data=[0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 28.0 29.0 30.0 31.0 32.0 33.0 34.0 35.0 36.0 37.0 38.0 39.0])
//TEST_INPUT: set sbuf = ubuffer(data=[0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 28.0 29.0 30.0 31.0 32.0 33.0 34.0 35.0 36.0 37.0 38.0 39.0], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0  0 0 0 0  0 0 0 0  0], stride=4)


// ===============================================================================
// Memory layout with std140 and input data [0.0, 1.0, ..., 39.0]
//
// column_major                                   row_major
// ---------------------------------              -----------------------------
//
// m1 (float4x4):                                 m1 (float4x4):
//       col0 col1 col2 col3                            col0 col1 col2 col3
// row0 [  0    4    8   12 ]                     row0 [  0    1    2    3 ]
// row1 [  1    5    9   13 ]                     row1 [  4    5    6    7 ]
// row2 [  2    6   10   14 ]                     row2 [  8    9   10   11 ]
// row3 [  3    7   11   15 ]                     row3 [ 12   13   14   15 ]
//
// m2 (float2x4):                                 m2 (float2x4):
//       col0 col1 col2 col3                            col0 col1 col2 col3
// row0 [ 16   20   24   28 ]                     row0 [ 16   17   18   19 ]
// row1 [ 17   21   25   29 ]                     row1 [ 20   21   22   23 ]
//        18   22   26   30
//        19   23   27   31
//
// m3 (float3x2):                                 m3 (float3x2):
//       col0 col1                                      col0 col1
// row0 [ 32   36 ]                               row0 [ 24   25 ] 26   27
// row1 [ 33   37 ]                               row1 [ 28   29 ] 30   31
// row2 [ 34   38 ]                               row2 [ 32   33 ] 34   35
//        35   39
//
// Offsets:                                       Offsets:
//   m1: 0, m2: 64, m3: 128                         m1: 0, m2: 64, m3: 96
//   Total: 160 bytes                               Total: 144 bytes
// ===============================================================================

// Verify struct member decorations
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 0 Offset 0
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 0 RowMajor
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 0 MatrixStride 16
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 1 Offset 64
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 1 RowMajor
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 1 MatrixStride 16
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 2 Offset 128
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 2 RowMajor
//CHECK_COLUMN: OpMemberDecorate %MatrixData{{.*}} 2 MatrixStride 16

//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 0 Offset 0
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 0 ColMajor
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 0 MatrixStride 16
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 1 Offset 64
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 1 ColMajor
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 1 MatrixStride 16
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 2 Offset 96
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 2 ColMajor
//CHECK_ROW: OpMemberDecorate %MatrixData{{.*}} 2 MatrixStride 16


// Verify OpTypeMatrix is used (not arrays of vectors)
// float4x4 m1:
//CHECK_COLUMN: %mat4v4float = OpTypeMatrix %v4float 4
//CHECK_ROW: %mat4v4float = OpTypeMatrix %v4float 4
// float2x4 m2:
//CHECK_COLUMN: %mat2v4float = OpTypeMatrix %v4float 2
//CHECK_ROW: %mat2v4float = OpTypeMatrix %v4float 2
// float3x2 m3:
//CHECK_COLUMN: %mat3v2float = OpTypeMatrix %v2float 3
//CHECK_ROW: %mat3v2float = OpTypeMatrix %v2float 3

//CHECK_COLUMN: %MatrixData{{.*}} = OpTypeStruct %mat4v4float %mat2v4float %mat3v2float  
//CHECK_ROW: %MatrixData{{.*}} = OpTypeStruct %mat4v4float %mat2v4float %mat3v2float  

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // m1 (float4x4) corners
    outputBuffer[0] = cbuf.m1[0][0];
    outputBuffer[1] = cbuf.m1[0][3];
    outputBuffer[2] = cbuf.m1[3][0];
    outputBuffer[3] = cbuf.m1[3][3];
    
    // m2 (float2x4) corners
    outputBuffer[4] = cbuf.m2[0][0];
    outputBuffer[5] = cbuf.m2[0][3];
    outputBuffer[6] = cbuf.m2[1][0];
    outputBuffer[7] = cbuf.m2[1][3];
    
    // m3 (float3x2) corners
    outputBuffer[8] = cbuf.m3[0][0];
    outputBuffer[9] = cbuf.m3[0][1];
    outputBuffer[10] = cbuf.m3[2][0];
    outputBuffer[11] = cbuf.m3[2][1];

    // check cbuf and sbuf match
    bool match =
        cbuf.m1[0][0] == sbuf.m1[0][0] &&
        cbuf.m1[0][3] == sbuf.m1[0][3] &&
        cbuf.m1[3][0] == sbuf.m1[3][0] &&
        cbuf.m1[3][3] == sbuf.m1[3][3] &&

        cbuf.m2[0][0] == sbuf.m2[0][0] &&
        cbuf.m2[0][3] == sbuf.m2[0][3] &&
        cbuf.m2[1][0] == sbuf.m2[1][0] &&
        cbuf.m2[1][3] == sbuf.m2[1][3] &&

        cbuf.m3[0][0] == sbuf.m3[0][0] &&
        cbuf.m3[0][1] == sbuf.m3[0][1] &&
        cbuf.m3[2][0] == sbuf.m3[2][0] &&
        cbuf.m3[2][1] == sbuf.m3[2][1];
    outputBuffer[12] = match ? 1.0f : 0.0f;
}


// Expected outputs for row_major matrix layout with std140 data layout, checking all matrix corners
// m1 (4x4) at offset 0, stride 4:
//CHECK_COMPUTE_ROW: 0.0
//CHECK_COMPUTE_ROW: 3.0
//CHECK_COMPUTE_ROW: 12.0
//CHECK_COMPUTE_ROW: 15.0
// m2 (2x4) at offset 64 (index 16), stride 4:
//CHECK_COMPUTE_ROW: 16.0
//CHECK_COMPUTE_ROW: 19.0
//CHECK_COMPUTE_ROW: 20.0
//CHECK_COMPUTE_ROW: 23.0
// m3 (3x2) at offset 96 (index 24), stride 4:
//CHECK_COMPUTE_ROW: 24.0
//CHECK_COMPUTE_ROW: 25.0
//CHECK_COMPUTE_ROW: 32.0
//CHECK_COMPUTE_ROW: 33.0
// RWStructuredBuffer and ConstantBuffer have the same layout
//CHECK_COMPUTE_ROW: 1.0

// Expected outputs for column_major matrix layout with std140 data layout, checking all matrix corners
// m1 (4x4) at offset 0, stride 4:
//CHECK_COMPUTE_COLUMN: 0.0
//CHECK_COMPUTE_COLUMN: 12.0
//CHECK_COMPUTE_COLUMN: 3.0
//CHECK_COMPUTE_COLUMN: 15.0
// m2 (2x4) at offset 64 (index 16), stride 4:
//CHECK_COMPUTE_COLUMN: 16.0
//CHECK_COMPUTE_COLUMN: 28.0
//CHECK_COMPUTE_COLUMN: 17.0
//CHECK_COMPUTE_COLUMN: 29.0
// m3 (3x2) at offset 128 (index 32), stride 4:
//CHECK_COMPUTE_COLUMN: 32.0
//CHECK_COMPUTE_COLUMN: 36.0
//CHECK_COMPUTE_COLUMN: 34.0
//CHECK_COMPUTE_COLUMN: 38.0
// RWStructuredBuffer and ConstantBuffer have the same layout
//CHECK_COMPUTE_COLUMN: 1.0
