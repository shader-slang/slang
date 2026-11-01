// An integer matrix in a buffer must use the offsets that reflection reports.
// A column-major matrix is stored as column vectors, and a row-major matrix as
// row vectors. Before the fix, `legalizeMatrixTypes` turned every integer matrix
// into row vectors before buffer lowering saw the matrix layout.

//TEST:SIMPLE(filecheck=COLMAJOR):-target spirv-asm -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=ROWMAJOR):-target spirv-asm -entry computeMain -stage compute -matrix-layout-row-major

struct S
{
    column_major int2x3 m;
    int2x3 k;
    row_major int2x3 r;
    column_major bool2x3 b;
    int tail;
}

RWStructuredBuffer<S> buffer;
RWStructuredBuffer<int> output;

// The member `k` has no qualifier, so it follows the default layout.
// COLMAJOR: OpMemberDecorate %S_std430 0 Offset 0
// COLMAJOR: OpMemberDecorate %S_std430 1 Offset 24
// COLMAJOR: OpMemberDecorate %S_std430 2 Offset 48
// COLMAJOR: OpMemberDecorate %S_std430 3 Offset 80
// COLMAJOR: OpMemberDecorate %S_std430 4 Offset 104

// Column vectors of `int2x3` have two elements, and row vectors have three.
// COLMAJOR-DAG: %_MatrixStorage_int2x3_ColMajorstd430 = OpTypeStruct %_arr_v2int_int_3
// COLMAJOR-DAG: %_MatrixStorage_int2x3std430 = OpTypeStruct %_arr_v3int_int_2
// COLMAJOR-DAG: %_MatrixStorage_bool2x3_ColMajorstd430 = OpTypeStruct %_arr_v2int_int_3

// With the row-major default, `k` is a row-major matrix. A row of three
// elements has a stride of 16 bytes, so `k` has 32 bytes and `m` is not changed.
// ROWMAJOR: OpMemberDecorate %S_std430 0 Offset 0
// ROWMAJOR: OpMemberDecorate %S_std430 1 Offset 32
// ROWMAJOR: OpMemberDecorate %S_std430 2 Offset 64
// ROWMAJOR: OpMemberDecorate %S_std430 3 Offset 96
// ROWMAJOR: OpMemberDecorate %S_std430 4 Offset 120

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = buffer[0].m[0][1] + buffer[0].k[0][1] + buffer[0].r[0][1] +
                (buffer[0].b[1][2] ? 1 : 0) + buffer[0].tail;
}
