// Regression for shader-slang/slang#13446: SPIR-V, GLSL, WGSL and Metal have no integer matrix
// types, so an integer matrix in a buffer is stored as an array of its columns when it is
// `column_major` and of its rows otherwise, and an element access picks the vector and the
// component in that order. The default layout decides for the unqualified `u`.

//TEST:SIMPLE(filecheck=SPV_COL):-target spirv-asm -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPV_ROW):-target spirv-asm -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=GLSL_COL):-target glsl -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=GLSL_ROW):-target glsl -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=WGSL_COL):-target wgsl -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=WGSL_ROW):-target wgsl -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=METAL_COL):-target metal -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL_ROW):-target metal -entry computeMain -stage compute -matrix-layout-row-major

struct Matrices
{
    column_major int2x3 m;
    column_major int2x3 a[2];
    row_major int2x3 r;
    column_major uint3x3 q;
    int2x3 u;
}

RWStructuredBuffer<Matrices> input;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input[0].m[0][1];
    output[1] = input[0].a[1][1][2];
    output[2] = input[0].r[1][2];
    output[3] = int(input[0].q[2][0]);
    output[4] = input[0].u[0][1];
}

// The member offsets are the ones reflection reports. In std430 a column of `int2x3` is an
// `int2` (8 bytes), so `m` takes 24 bytes; a row is an `int3` with 16-byte alignment, so `r`
// starts at 80 and takes 32 bytes.
// SPV_COL-DAG: OpMemberDecorate %Matrices_std430 0 Offset 0
// SPV_COL-DAG: OpMemberDecorate %Matrices_std430 1 Offset 24
// SPV_COL-DAG: OpMemberDecorate %Matrices_std430 2 Offset 80
// SPV_COL-DAG: OpMemberDecorate %Matrices_std430 3 Offset 112
// SPV_COL-DAG: OpMemberDecorate %Matrices_std430 4 Offset 160
// SPV_COL-DAG: OpDecorate %_arr_v2int_int_3 ArrayStride 8
// SPV_COL-DAG: OpDecorate %_arr__MatrixStorage_int2x3_ColMajorstd430_int_2 ArrayStride 24
// SPV_COL-DAG: OpDecorate %_arr_v3int_int_2 ArrayStride 16
// SPV_COL-DAG: %Matrices_std430 = OpTypeStruct %_MatrixStorage_int2x3_ColMajorstd430 %_Array_std430_matrix_int_2_3_2 %_MatrixStorage_int2x3std430 %_MatrixStorage_uint3x3_ColMajorstd430 %_MatrixStorage_int2x3_ColMajorstd430

// Under the row-major default `u` is stored as rows.
// SPV_ROW-DAG: OpMemberDecorate %Matrices_std430 4 Offset 160
// SPV_ROW-DAG: %Matrices_std430 = OpTypeStruct %_MatrixStorage_int2x3_ColMajorstd430 %_Array_std430_matrix_int_2_3_2 %_MatrixStorage_int2x3std430 %_MatrixStorage_uint3x3_ColMajorstd430 %_MatrixStorage_int2x3std430

// GLSL_COL: struct _MatrixStorage_int2x3_ColMajornatural
// GLSL_COL-NEXT: {
// GLSL_COL-NEXT: ivec2 data{{.*}}[3];
// GLSL_COL: struct _MatrixStorage_int2x3natural
// GLSL_COL-NEXT: {
// GLSL_COL-NEXT: ivec3 data{{.*}}[2];
// GLSL_COL: struct Matrices
// GLSL_COL-NEXT: {
// GLSL_COL-NEXT: _MatrixStorage_int2x3_ColMajornatural{{.*}} m
// GLSL_COL-NEXT: a
// GLSL_COL-NEXT: _MatrixStorage_int2x3natural{{.*}} r
// GLSL_COL-NEXT: _MatrixStorage_uint3x3_ColMajornatural{{.*}} q
// GLSL_COL-NEXT: _MatrixStorage_int2x3_ColMajornatural{{.*}} u
// GLSL_COL: .m_0.data_{{[0-9]+}}[1][0];
// GLSL_COL: .a_0.data_{{[0-9]+}}[1].data_{{[0-9]+}}[2][1];
// GLSL_COL: .r_0.data_{{[0-9]+}}[1][2];
// GLSL_COL: .q_0.data_{{[0-9]+}}[0][2]
// GLSL_COL: .u_0.data_{{[0-9]+}}[1][0];

// GLSL_ROW: struct Matrices
// GLSL_ROW: _MatrixStorage_int2x3natural{{.*}} u
// GLSL_ROW: .u_0.data_{{[0-9]+}}[0][1];

// WGSL_COL: struct _MatrixStorage_int2x3_ColMajorstd430
// WGSL_COL-NEXT: {
// WGSL_COL-NEXT: data{{.*}} : array<vec2<i32>, i32(3)>
// WGSL_COL: .m_0.data_{{[0-9]+}}[i32(1)][i32(0)];
// WGSL_COL: .a_0.data_{{[0-9]+}}[i32(1)].data_{{[0-9]+}}[i32(2)][i32(1)];
// WGSL_COL: .r_0.data_{{[0-9]+}}[i32(1)][i32(2)];
// WGSL_COL: .q_0.data_{{[0-9]+}}[i32(0)][i32(2)]
// WGSL_COL: .u_0.data_{{[0-9]+}}[i32(1)][i32(0)];

// WGSL_ROW: .u_0.data_{{[0-9]+}}[i32(0)][i32(1)];

// Metal device buffers store the rows or columns as packed vectors.
// METAL_COL: struct _MatrixStorage_int2x3_ColMajornatural
// METAL_COL-NEXT: {
// METAL_COL-NEXT: array<packed_int2, int(3)> data
// METAL_COL: ->m_0)->data_{{[0-9]+}}[int(1)][int(0)];
// METAL_COL: ->a_0)->data_{{[0-9]+}}[int(1)])->data_{{[0-9]+}}[int(2)][int(1)];
// METAL_COL: ->r_0)->data_{{[0-9]+}}[int(1)][int(2)];
// METAL_COL: ->q_0)->data_{{[0-9]+}}[int(0)][int(2)]
// METAL_COL: ->u_0)->data_{{[0-9]+}}[int(1)][int(0)];

// METAL_ROW: ->m_0)->data_{{[0-9]+}}[int(1)][int(0)];
// METAL_ROW: ->u_0)->data_{{[0-9]+}}[int(0)][int(1)];
