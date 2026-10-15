// Regression for shader-slang/slang#13446: an integer matrix in a structured buffer is stored with
// the layout that reflection reports, `column_major` included. SPIR-V, GLSL, WGSL and Metal have no
// integer matrix types, so these matrices are stored as arrays of row or column vectors, and every
// read and write must address the same words reflection describes.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-row-major

// Element `[r][c]` of array element `k` holds `base + 100*k + 10*r + c`, at the word reflection
// gives for the member's layout. Every member has an explicit layout, so the default layout does
// not move any word; std430 and natural layout need no padding here, so one data set serves every
// leg, CPU included.
struct Matrices
{
    column_major int2x3 m;
    column_major int2x3 a[2];
    row_major int3x2 r;
    column_major int4x4 q;
    column_major uint2x4 u;
}

// clang-format off
//TEST_INPUT:ubuffer(data=[100 110 101 111 102 112 200 210 201 211 202 212 300 310 301 311 302 312 400 401 410 411 420 421 500 510 520 530 501 511 521 531 502 512 522 532 503 513 523 533 600 610 601 611 602 612 603 613], stride=192):name input
// clang-format on
RWStructuredBuffer<Matrices> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[noinline]
int sumOfRow(int2x3 m, int row)
{
    return m[row][0] + m[row][1] + m[row][2];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input[0].m[0][1];
    output[1] = input[0].m[1][2];
    output[2] = input[0].a[1][0][2];
    output[3] = input[0].r[2][1];
    output[4] = input[0].q[1][3];
    output[5] = input[0].q[3][1];
    output[6] = int(input[0].u[1][3]);
    output[7] = input[0].a[0][1][1];

    // A row of a column-major matrix gathers one element from each column.
    int3 row = input[0].m[1];
    output[8] = row.x;
    output[9] = row.z;

    int2x3 whole = input[0].a[1];
    output[10] = whole[0][1];
    output[11] = sumOfRow(input[0].a[0], 1);
    int4x4 square = input[0].q;
    output[12] = square[2][3];

    // Writes go through the same storage layout; read them back.
    input[0].m[0][2] = 1002;
    input[0].a[0] = int2x3(2000, 2001, 2002, 2010, 2011, 2012);
    input[0].r[1] = int2(4010, 4011);
    input[0].m[1] = int3(1110, 1111, 1112);
    output[13] = input[0].m[0][2];
    output[14] = input[0].a[0][1][0];
    output[15] = input[0].a[0][0][2];
    output[16] = input[0].r[1][1];
    output[17] = input[0].m[1][2];
    output[18] = input[0].a[1][1][1];
    output[19] = input[0].r[2][0];
    output[20] = input[0].m[1][0];
    output[21] = input[0].m[0][1];
}

// CHECK: type: int32_t
// CHECK-NEXT: 101
// CHECK-NEXT: 112
// CHECK-NEXT: 302
// CHECK-NEXT: 421
// CHECK-NEXT: 513
// CHECK-NEXT: 531
// CHECK-NEXT: 613
// CHECK-NEXT: 211
// CHECK-NEXT: 110
// CHECK-NEXT: 112
// CHECK-NEXT: 301
// CHECK-NEXT: 633
// CHECK-NEXT: 523
// CHECK-NEXT: 1002
// CHECK-NEXT: 2010
// CHECK-NEXT: 2002
// CHECK-NEXT: 4011
// CHECK-NEXT: 1112
// CHECK-NEXT: 311
// CHECK-NEXT: 420
// CHECK-NEXT: 1110
// CHECK-NEXT: 101
