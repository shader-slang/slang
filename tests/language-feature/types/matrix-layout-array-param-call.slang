// Regression for shader-slang/slang#13375: a traditional-syntax parameter declared as an array of
// matrices with an explicit layout accepts a plain array or a buffer array of another layout by
// value. The argument converts element by element (shader-slang/slang#13376), so the logical
// values survive. Other modifiers on array parameters (`no_diff`, `unorm`) stay on the parameter
// and keep accepting plain arrays. A forward declaration and a definition whose layouts differ are
// one function for now (shader-slang/slang#13383): the call uses the declaration's layout and the
// body reads its own, by value and for a `__ref`/`__constref` buffer element.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name output
RWStructuredBuffer<float> output;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=32):name stored
RWStructuredBuffer<row_major float2x3> stored;

struct ColumnMajorMatrices
{
    column_major float2x3 values[2];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=48):name columnMajorStorage
RWStructuredBuffer<ColumnMajorMatrices> columnMajorStorage;

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

float countDeclaredRowMajorMismatches(row_major float2x3 m[2]);
float readDeclaredRowMajor(row_major float2x3 m);
void addHundredDeclaredRowMajor(__ref row_major float2x3 m);
float readBorrowedDeclaredRowMajor(__constref row_major float2x3 m);

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

    for (int k = 0; k < 2; k++)
        columnMajorStorage[0].values[k] =
            float2x3(1 + 6 * k, 2 + 6 * k, 3 + 6 * k, 4 + 6 * k, 5 + 6 * k, 6 + 6 * k);
    output[2] = countRowMajorMismatches(columnMajorStorage[0].values);
    output[3] = countColumnMajorMismatches(columnMajorStorage[0].values);

    float coefficients[2] = { 2.0, 3.0 };
    var x = diffPair(5.0);
    bwd_diff(scaleAndOffset)(coefficients, x, 1.0);
    output[4] = scaleAndOffset(coefficients, 5.0);
    output[5] = x.d;

    float4 colors[2] = { float4(0.25), float4(0.5) };
    output[6] = readUNorm(colors);

    float2x3 fresh[2];
    for (int k = 0; k < 2; k++)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 3; c++)
                fresh[k][r][c] = 1 + 6 * k + 3 * r + c;
    output[7] = countDeclaredRowMajorMismatches(fresh);
    output[8] = readDeclaredRowMajor(fresh[0]);

    // `[0][1]` holds 2 and `[1][0]` holds 4. The `column_major` bodies must read and write the
    // same logical elements of the `row_major` storage.
    stored[0] = float2x3(1, 2, 3, 4, 5, 6);
    addHundredDeclaredRowMajor(stored[0]);
    output[9] = stored[0][0][1];
    output[10] = readBorrowedDeclaredRowMajor(stored[0]);
}

float countDeclaredRowMajorMismatches(column_major float2x3 m[2])
{
    return countMismatches(m);
}

float readDeclaredRowMajor(column_major float2x3 m)
{
    return m[1][2];
}

void addHundredDeclaredRowMajor(__ref column_major float2x3 m)
{
    m[0][1] += 100;
}

float readBorrowedDeclaredRowMajor(__constref column_major float2x3 m)
{
    return m[1][0];
}

// CHECK: type: float
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 13.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 0.5
// CHECK-NEXT: 0.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 102.0
// CHECK-NEXT: 4.0
