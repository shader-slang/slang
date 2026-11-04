// regression for #13582: a matrix whose layout differs from the parameter's works as an inout/out argument

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major -xslang -DELEMENT=int
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major -xslang -DELEMENT=int
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

#ifndef ELEMENT
#define ELEMENT float
#endif

typealias M = matrix<ELEMENT, 2, 4>;

struct S
{
    row_major M rowInOut;
    column_major M columnInOut;
    row_major M rowOut;
    column_major M columnOut;
    row_major M rowArray[2];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=192):name sb
RWStructuredBuffer<S> sb;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<float> output;

// logical values m[r][c] == base + 4*r + c
M make(ELEMENT base)
{
    M m;
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 4; c++)
            m[r][c] = base + ELEMENT(4 * r + c);
    return m;
}

void bump(inout M x)
{
    x[0][1] += ELEMENT(100);
    x[1][2] += ELEMENT(200);
}

void bumpRowMajor(inout row_major M x)
{
    x[0][1] += ELEMENT(100);
    x[1][2] += ELEMENT(200);
}

void fill(out M x) { x = make(ELEMENT(50)); }

void bumpArray(inout M x[2]) { x[1][0][1] += ELEMENT(100); }

[numthreads(1, 1, 1)]
void computeMain()
{
    sb[0].rowInOut = make(ELEMENT(1));
    sb[0].columnInOut = make(ELEMENT(1));
    sb[0].rowArray[0] = make(ELEMENT(1));
    sb[0].rowArray[1] = make(ELEMENT(1));

    bump(sb[0].rowInOut);
    bump(sb[0].columnInOut);
    fill(sb[0].rowOut);
    fill(sb[0].columnOut);
    bumpArray(sb[0].rowArray);

    // CHECK: type: float
    // CHECK-NEXT: 102.0
    // CHECK-NEXT: 207.0
    // CHECK-NEXT: 1.0
    output[0] = float(sb[0].rowInOut[0][1]);
    output[1] = float(sb[0].rowInOut[1][2]);
    output[2] = float(sb[0].rowInOut[0][0]);

    // CHECK-NEXT: 102.0
    // CHECK-NEXT: 207.0
    // CHECK-NEXT: 1.0
    output[3] = float(sb[0].columnInOut[0][1]);
    output[4] = float(sb[0].columnInOut[1][2]);
    output[5] = float(sb[0].columnInOut[0][0]);

    // CHECK-NEXT: 51.0
    // CHECK-NEXT: 56.0
    // CHECK-NEXT: 51.0
    // CHECK-NEXT: 56.0
    output[6] = float(sb[0].rowOut[0][1]);
    output[7] = float(sb[0].rowOut[1][2]);
    output[8] = float(sb[0].columnOut[0][1]);
    output[9] = float(sb[0].columnOut[1][2]);

    // CHECK-NEXT: 102.0
    // CHECK-NEXT: 2.0
    output[10] = float(sb[0].rowArray[1][0][1]);
    output[11] = float(sb[0].rowArray[0][0][1]);

    // CHECK-NEXT: 102.0
    // CHECK-NEXT: 207.0
    row_major M rowLocal = make(ELEMENT(1));
    bump(rowLocal);
    output[12] = float(rowLocal[0][1]);
    output[13] = float(rowLocal[1][2]);

    // CHECK-NEXT: 102.0
    // CHECK-NEXT: 207.0
    M plainLocal = make(ELEMENT(1));
    bumpRowMajor(plainLocal);
    output[14] = float(plainLocal[0][1]);
    output[15] = float(plainLocal[1][2]);
}
