// Regression for shader-slang/slang#13446: a row of a `column_major` matrix with one column is a
// scalar, and storing it writes one element of that column at the word reflection gives. CPU and
// CUDA do not accept matrices with one column, so Vulkan checks the values and Metal and WGSL check
// that the stores compile.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=COMPILES):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=COMPILES):-target wgsl -entry computeMain -stage compute

// `a` is one `int3` column (16 bytes); `b` is one `bool3` column stored as integers.
struct Narrow
{
    column_major int3x1 a;
    column_major bool3x1 b;
}

//TEST_INPUT:ubuffer(data=[1 2 3 0 0 0 0 0], stride=32):name input
RWStructuredBuffer<Narrow> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    input[0].a[1] = 50;
    input[0].a[2] += 10;
    input[0].b[2] = true;

    output[0] = input[0].a[0][0];
    output[1] = input[0].a[1][0];
    output[2] = input[0].a[2][0];
    output[3] = input[0].b[0][0] ? 1 : 0;
    output[4] = input[0].b[1][0] ? 1 : 0;
    output[5] = input[0].b[2][0] ? 1 : 0;
}

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 50
// CHECK-NEXT: 13
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1

// COMPILES: computeMain
