// Regression for shader-slang/slang#13446: a swizzled store into a row of an integer matrix in a
// buffer writes each selected element at the word reflection gives for it, in both layouts and
// for a `bool` matrix, whose elements are stored as integers on SPIR-V.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

struct Matrices
{
    row_major int2x3 r;
    column_major int2x3 c;
}

struct Bools
{
    column_major bool2x3 b;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=56):name input
RWStructuredBuffer<Matrices> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=24):name bools
RWStructuredBuffer<Bools> bools;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    input[0].r[1].xz = int2(7, 8);
    input[0].c[1].xz = int2(5, 6);
    input[0].c[0].zy += int2(1, 2);
    bools[0].b[1].xz = bool2(true, true);

    output[0] = input[0].r[1][0];
    output[1] = input[0].r[1][1];
    output[2] = input[0].r[1][2];
    output[3] = input[0].c[1][0];
    output[4] = input[0].c[1][1];
    output[5] = input[0].c[1][2];
    output[6] = input[0].c[0][1];
    output[7] = input[0].c[0][2];
    output[8] = bools[0].b[1][0] ? 1 : 0;
    output[9] = bools[0].b[1][1] ? 1 : 0;
    output[10] = bools[0].b[1][2] ? 1 : 0;
    output[11] = bools[0].b[0][0] ? 1 : 0;
    output[12] = input[0].r[0][0];
}

// CHECK: type: int32_t
// CHECK-NEXT: 7
// CHECK-NEXT: 0
// CHECK-NEXT: 8
// CHECK-NEXT: 5
// CHECK-NEXT: 0
// CHECK-NEXT: 6
// CHECK-NEXT: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0

// In column-major storage `c[1].xz` writes element 1 of columns 0 and 2.
// METAL: ->c_0)->data_{{[0-9]+}}[int(0)][int(1)] = int(5);
// METAL: ->c_0)->data_{{[0-9]+}}[int(2)][int(1)] = int(6);
// WGSL: .c_0.data_{{[0-9]+}}[i32(0)][i32(1)] = i32(5);
// WGSL: .c_0.data_{{[0-9]+}}[i32(2)][i32(1)] = i32(6);
