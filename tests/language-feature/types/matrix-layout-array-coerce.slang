// Regression for shader-slang/slang#13376: an array of matrices with an explicit `row_major` or
// `column_major` layout converts to an array of matrices with a different layout, both when it is
// passed to a function and when it is assigned. The layout only describes how a matrix is stored,
// so the conversion is element by element and keeps the logical values.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

// Matrix `k` of each array holds the logical values `m[r][c] == 1 + 8*k + 4*r + c`. The
// row-major members store them row by row and the column-major member column by column. A
// `float2x4` row or column needs no padding in any buffer layout, so one data set serves every
// target and both default layouts.
struct Matrices
{
    row_major float2x4 rowMajor[2];
    column_major float2x4 columnMajor[2];
    row_major float2x4 nested[2][2];
}

// clang-format off
//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0  1.0 5.0 2.0 6.0 3.0 7.0 4.0 8.0 9.0 13.0 10.0 14.0 11.0 15.0 12.0 16.0  1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 28.0 29.0 30.0 31.0 32.0], stride=256):name input
// clang-format on
StructuredBuffer<Matrices> input;

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name output
RWStructuredBuffer<float> output;

// Return how many elements of `m` differ from the logical values `first + 8*k + 4*r + c`.
float countMismatches(float2x4 m[2], float first)
{
    float mismatches = 0;
    for (int k = 0; k < 2; k++)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 4; c++)
                if (m[k][r][c] != first + 8 * k + 4 * r + c)
                    mismatches++;
    return mismatches;
}

func countMismatchesModern(m: row_major float2x4[2]) -> float { return countMismatches(m, 1); }

float countMismatchesUnsized(float2x4 m[], int count)
{
    float mismatches = 0;
    for (int k = 0; k < count; k++)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 4; c++)
                if (m[k][r][c] != 1 + 8 * k + 4 * r + c)
                    mismatches++;
    return mismatches;
}

void addHundred(inout float2x4 m[2]) { m[0][0][0] += 100; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // Passing to a plain array parameter.
    output[0] = countMismatches(input[0].rowMajor, 1);
    output[1] = countMismatches(input[0].columnMajor, 1);

    float2x4 local[2] = input[0].rowMajor;
    output[2] = countMismatches(local, 1);
    local = input[0].columnMajor;
    output[3] = countMismatches(local, 1);

    // A modern-syntax parameter with an explicit layout, called with a plain array.
    output[4] = countMismatchesModern(local);

    // Nested arrays convert element by element at every level.
    float2x4 nested[2][2] = input[0].nested;
    output[5] = countMismatches(nested[1], 17);

    // A sized array passed to an unsized plain parameter.
    output[6] = countMismatchesUnsized(input[0].rowMajor, 2);

    // An `inout` argument is converted in and back out again.
    row_major float2x4 rowMajorLocal[2] = input[0].columnMajor;
    addHundred(rowMajorLocal);
    output[7] = rowMajorLocal[0][0][0];
    output[8] = rowMajorLocal[1][1][3];
}

// CHECK: type: float
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 101.0
// CHECK-NEXT: 16.0
