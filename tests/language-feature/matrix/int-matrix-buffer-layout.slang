// Integer and bool matrices in buffers must follow the matrix layout that
// reflection reports. The test reads each element with the host layout that
// reflection gives for a column-major matrix, so a wrong storage layout in
// the generated code returns a different word.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -emit-spirv-directly -xslang -matrix-layout-row-major

struct IntMatrices
{
    column_major int2x3 m;
    column_major int2x3 n[2];
    column_major int3x3 q;
    int tail;
}

struct BoolMatrices
{
    column_major bool2x3 b;
    int tail;
}

// Word i holds the value i + 1.
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36], stride=4):name intBuffer
RWStructuredBuffer<IntMatrices> intBuffer;

//TEST_INPUT:ubuffer(data=[1 0 0 1 1 0 7 0], stride=4):name boolBuffer
RWStructuredBuffer<BoolMatrices> boolBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    let ints = intBuffer[0];

    // `m` is at word 0. A column occupies two words, so m[r][c] is word 2 * c + r.
    output[0] = ints.m[0][1];
    // CHECK: type: int32_t
    // CHECK-NEXT: 3
    output[1] = ints.m[1][2];
    // CHECK-NEXT: 6

    // `n` starts at word 6. Each element of the array is six words.
    output[2] = ints.n[1][0][1];
    // CHECK-NEXT: 15
    output[3] = ints.n[0][1][0];
    // CHECK-NEXT: 8

    // A matrix that is copied as a whole must keep its layout.
    int2x3 copy = ints.n[1];
    output[4] = copy[1][2];
    // CHECK-NEXT: 18

    // `q` is at word 20. A column of an `int3x3` occupies four words.
    output[5] = ints.q[1][0];
    // CHECK-NEXT: 22
    output[6] = ints.q[0][1];
    // CHECK-NEXT: 25

    // Members after the matrices must be at the offset that reflection reports.
    output[7] = ints.tail;
    // CHECK-NEXT: 33

    // `b` is at word 0. A column occupies two words, so b[r][c] is word 2 * c + r.
    let bools = boolBuffer[0];
    output[8] = bools.b[0][0] ? 1 : 0;
    // CHECK-NEXT: 1
    output[9] = bools.b[1][0] ? 1 : 0;
    // CHECK-NEXT: 0
    output[10] = bools.b[0][1] ? 1 : 0;
    // CHECK-NEXT: 0
    output[11] = bools.b[1][1] ? 1 : 0;
    // CHECK-NEXT: 1
    output[12] = bools.b[0][2] ? 1 : 0;
    // CHECK-NEXT: 1
    output[13] = bools.b[1][2] ? 1 : 0;
    // CHECK-NEXT: 0
    output[14] = bools.tail;
    // CHECK-NEXT: 7
}
