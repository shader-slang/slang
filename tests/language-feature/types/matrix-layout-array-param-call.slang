// Regression for shader-slang/slang#13375: a traditional-syntax parameter declared as an array of
// matrices with an explicit layout accepts a plain array argument, by value and by `inout`. The
// argument converts element by element (shader-slang/slang#13376), so the logical values survive.
// Other modifiers on array parameters (`no_diff`, `unorm`) stay on the parameter and keep
// accepting plain arrays. A forward declaration and a definition whose layouts differ are one
// function: the call converts to the declaration's layout and the body reads its own, as in HLSL.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name output
RWStructuredBuffer<float> output;

// Matrix `k` holds the logical values `m[r][c] == 1 + 6*k + 3*r + c`.
float countMismatches(float2x3 m[2])
{
    float mismatches = 0;
    for (int k = 0; k < 2; k++)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 3; c++)
                if (m[k][r][c] != 1 + 6 * k + 3 * r + c)
                    mismatches++;
    return mismatches;
}

float countRowMajorMismatches(row_major float2x3 m[2])
{
    return countMismatches(m);
}

float countColumnMajorMismatches(column_major float2x3 m[2])
{
    return countMismatches(m);
}

void addHundred(inout row_major float2x3 m[2])
{
    m[1][1][0] += 100;
}

float countDeclaredRowMajorMismatches(row_major float2x3 m[2]);
float readDeclaredRowMajor(row_major float2x3 m);

[Differentiable]
float scaleAndOffset(no_diff float coefficients[2], float x)
{
    return coefficients[0] * x + coefficients[1];
}

float readUNorm(unorm float4 v[2])
{
    return v[1].y;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x3 plain[2];
    for (int k = 0; k < 2; k++)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 3; c++)
                plain[k][r][c] = 1 + 6 * k + 3 * r + c;

    output[0] = countRowMajorMismatches(plain);
    output[1] = countColumnMajorMismatches(plain);

    // The `inout` copy-back keeps the other elements: only `[1][1][0]` (logical 10) changes.
    addHundred(plain);
    output[2] = plain[1][1][0];
    output[3] = plain[1][1][2];

    float coefficients[2] = { 2.0, 3.0 };
    var x = diffPair(5.0);
    bwd_diff(scaleAndOffset)(coefficients, x, 1.0);
    output[4] = scaleAndOffset(coefficients, 5.0);
    output[5] = x.d;

    float4 colors[2] = { float4(0.25), float4(0.5) };
    output[6] = readUNorm(colors);

    // `plain[0]` was not changed by `addHundred`, so the mismatch count covers only `plain[1]`.
    output[7] = countDeclaredRowMajorMismatches(plain);
    output[8] = readDeclaredRowMajor(plain[0]);
}

float countDeclaredRowMajorMismatches(column_major float2x3 m[2])
{
    return countMismatches(m);
}

float readDeclaredRowMajor(column_major float2x3 m)
{
    return m[1][2];
}

// CHECK: type: float
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 110.0
// CHECK-NEXT: 12.0
// CHECK-NEXT: 13.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 0.5
// CHECK-NEXT: 1.0
// CHECK-NEXT: 6.0
