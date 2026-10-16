// Regression for shader-slang/slang#13446: an integer matrix in a constant buffer is stored with the
// layout that reflection reports, `column_major` included. Under std140 every row or column vector
// of the storage array starts on a 16-byte boundary, so a non-square `column_major int2x3`
// occupies three 16-byte columns and the members after it start where reflection says they do.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-row-major

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=WGSLSPIRV):-target wgsl-spirv-asm -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -matrix-layout-row-major

// Element `[r][c]` of array element `k` holds `base + 100*k + 10*r + c`, at the word reflection
// gives for the member's std140 layout; padding words hold 0. Every member has an explicit layout,
// so the default layout does not move any word. The CPU target lays constant buffers out without
// std140 padding, so this data set is only valid for the Vulkan legs.
struct Matrices
{
    column_major int3x3 q;
    column_major int2x3 m;
    column_major int2x3 a[2];
    row_major int2x3 r;
    column_major uint2x2 u;
}

// clang-format off
//TEST_INPUT:cbuffer(data=[500 510 520 0 501 511 521 0 502 512 522 0 100 110 0 0 101 111 0 0 102 112 0 0 200 210 0 0 201 211 0 0 202 212 0 0 300 310 0 0 301 311 0 0 302 312 0 0 400 401 402 0 410 411 412 0 600 610 0 0 601 611 0 0]):name input
// clang-format on
ConstantBuffer<Matrices> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[noinline]
int sumOfColumn(int2x3 m, int column)
{
    return m[0][column] + m[1][column];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input.q[0][1];
    output[1] = input.q[1][0];
    output[2] = input.q[2][1];
    output[3] = input.m[0][1];
    output[4] = input.m[1][2];
    output[5] = input.a[1][0][2];
    output[6] = input.r[1][2];
    output[7] = int(input.u[1][0]);

    int3 row = input.m[1];
    output[8] = row.x + row.y + row.z;

    int2x3 whole = input.a[1];
    output[9] = whole[1][0];
    output[10] = sumOfColumn(input.a[0], 2);
    int3x3 square = input.q;
    output[11] = square[2][0];
    output[12] = square[0][2];
    output[13] = input.r[0][0];
    output[14] = input.r[1].z;
}

// CHECK: type: int32_t
// CHECK-NEXT: 501
// CHECK-NEXT: 510
// CHECK-NEXT: 521
// CHECK-NEXT: 101
// CHECK-NEXT: 112
// CHECK-NEXT: 302
// CHECK-NEXT: 412
// CHECK-NEXT: 610
// CHECK-NEXT: 333
// CHECK-NEXT: 310
// CHECK-NEXT: 414
// CHECK-NEXT: 520
// CHECK-NEXT: 502
// CHECK-NEXT: 400
// CHECK-NEXT: 412

// The std140 offsets are the ones reflection reports: each column of `q`, `m` and `a[k]` and each
// row of `r` is a 16-byte vector.
// SPV-DAG: OpMemberDecorate %Matrices_std140 1 Offset 48
// SPV-DAG: OpMemberDecorate %Matrices_std140 2 Offset 96
// SPV-DAG: OpMemberDecorate %Matrices_std140 3 Offset 192
// SPV-DAG: OpMemberDecorate %Matrices_std140 4 Offset 224
// SPV-DAG: OpDecorate %_arr__MatrixStorage_int2x3_ColMajorstd140_int_2 ArrayStride 48
// SPV-DAG: OpDecorate %_arr_v2int_int_3 ArrayStride 16

// WGSL: data{{.*}} : array<vec4<i32>, i32(3)>
// WGSL: .q_0.data_{{[0-9]+}}[i32(1)][i32(0)];
// WGSL: .m_0.data_{{[0-9]+}}[i32(2)][i32(1)];
// WGSL: .a_0.data_{{[0-9]+}}[i32(1)].data_{{[0-9]+}}[i32(2)][i32(0)];
// WGSL: .r_0.data_{{[0-9]+}}[i32(1)][i32(2)];

// A whole row of `r` is a padded `vec4` in storage; reading it as an `int3` takes `.xyz`.
// WGSL: = {{.*}}.r_0.data_{{[0-9]+}}[i32(1)].xyz;

// WGSLSPIRV: OpEntryPoint

// A Metal constant buffer keeps native vectors: `int2` columns, so `m` is 24 bytes.
// METAL: array<int2, int(3)> data
// METAL: ->q_0)->data_{{[0-9]+}}[int(1)].x;
// METAL: ->m_{{[0-9]+}})->data_{{[0-9]+}}[int(2)].y;
// METAL: ->r_0[int(1)].z;
