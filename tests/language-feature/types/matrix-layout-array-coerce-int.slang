// Regression for shader-slang/slang#13376: an integer matrix array in a buffer, passed to an `inout`
// parameter of another layout, is converted in and written back in its own layout.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
// The Vulkan legs are expected failures because of shader-slang/slang#13446: SPIR-V lays out an
// integer matrix as row vectors whatever its layout, so it misreads this column-major data.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

// Integer matrix `k` holds `n[r][c] == 1 + 6*k + 3*r + c`, stored column by column.
struct IntMatrices
{
    column_major int2x3 n[2];
}

//TEST_INPUT:ubuffer(data=[1 4 2 5 3 6 7 10 8 11 9 12], stride=48):name intBuffer
RWStructuredBuffer<IntMatrices> intBuffer;

//TEST_INPUT:ubuffer(data=[-1 -1 -1], stride=4):out,name output
RWStructuredBuffer<float> output;

void addHundredInt(inout int2x3 n[2]) { n[1][0][2] += 100; }

[numthreads(1, 1, 1)]
void computeMain()
{
    addHundredInt(intBuffer[0].n);

    // The written element changes and its neighbours keep their values.
    output[0] = intBuffer[0].n[1][0][2];
    output[1] = intBuffer[0].n[1][0][1];
    output[2] = intBuffer[0].n[1][1][0];
}

// CHECK: type: float
// CHECK-NEXT: 109.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 10.0
