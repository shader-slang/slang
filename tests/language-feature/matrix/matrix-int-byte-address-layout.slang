// Regression for shader-slang/slang#13446: `ByteAddressBuffer.Load<int2x3>` and `Store<int2x3>`
// use the layout of the matrix type, which for this unqualified type is the column-major default
// here, as they already did for `float2x3`. Word `i` of `input` holds `i + 1`.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=COL):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=COL):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6], stride=4):name input
RWByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name stored
RWByteAddressBuffer stored;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    int2x3 m = input.Load<int2x3>(0);
    float2x3 f = input.Load<float2x3>(0);
    output[0] = m[0][1];
    output[1] = m[1][0];
    output[2] = asint(f[0][1]);
    output[3] = asint(f[1][0]);
    stored.Store<int2x3>(0, int2x3(10, 11, 12, 13, 14, 15));
}

// `[0][1]` is word 2 and `[1][0]` is word 1, for `int` and `float` alike, and the store writes
// the columns (10, 13), (11, 14), (12, 15); the byte-address buffer prints in hexadecimal.
// COL: A
// COL-NEXT: D
// COL-NEXT: B
// COL-NEXT: E
// COL-NEXT: C
// COL-NEXT: F
// COL-NEXT: type: int32_t
// COL-NEXT: 3
// COL-NEXT: 2
// COL-NEXT: 3
// COL-NEXT: 2
