//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target spirv
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target glsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target metal
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target wgsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cuda
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cpp
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target spirv -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target hlsl -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target glsl -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target metal -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target wgsl -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cuda -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cpp -lang hlsl
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target spirv -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target hlsl -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target glsl -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target metal -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target wgsl -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cuda -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cpp -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target spirv -DTEST_FLOAT=double
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target hlsl -DTEST_FLOAT=double
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target glsl -DTEST_FLOAT=double
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cuda -DTEST_FLOAT=double
//TEST:SIMPLE(filecheck=NATIVE): -entry computeMain -stage compute -target cpp -DTEST_FLOAT=double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -output-using-type -Xslang -DTEST_FLOAT=double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -slang -shaderobj -mtl -output-using-type

// NATIVE: {{[Mm]odf}}

// We check the returned fractions and integer outputs independently, using negative values
// to distinguish truncation toward zero from flooring. The matrix outputs also check both
// explicit layouts, and floating-point outputs check that the floating-output overloads resolve.
#ifndef TEST_FLOAT
#define TEST_FLOAT float
#endif

typedef vector<TEST_FLOAT, 3> TestVector;
typedef matrix<TEST_FLOAT, 2, 3> TestMatrix;

//TEST_INPUT:ubuffer(data=[-2.75 2.75 -0.5 0.5 -3.0 0.0], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// BUF: type: float
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}0.750000{{$}}
// BUF-NEXT: {{^}}-0.500000{{$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}2.000000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}0.750000{{$}}
// BUF-NEXT: {{^}}-0.500000{{$}}
// BUF-NEXT: {{^}}0.500000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}2.000000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-3.000000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}0.750000{{$}}
// BUF-NEXT: {{^}}-0.500000{{$}}
// BUF-NEXT: {{^}}0.500000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}2.000000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^-?0\.000000$}}
// BUF-NEXT: {{^}}-3.000000{{$}}
// BUF-NEXT: {{^-?0\.000000$}}

// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}-0.750000{{$}}
// BUF-NEXT: {{^}}-2.000000{{$}}
// BUF-NEXT: {{^}}1.000000{{$}}
// BUF-NEXT: {{^}}1.000000{{$}}
// BUF-NEXT: {{^}}1.000000{{$}}

/// Write the fractional and whole parts for scalar, vector, and matrix calls.
[numthreads(1, 1, 1)]
void computeMain()
{
    TestMatrix x = TestMatrix(
        TEST_FLOAT(inputBuffer[0]), TEST_FLOAT(inputBuffer[1]), TEST_FLOAT(inputBuffer[2]),
        TEST_FLOAT(inputBuffer[3]), TEST_FLOAT(inputBuffer[4]), TEST_FLOAT(inputBuffer[5]));

    int scalarWhole;
    TEST_FLOAT scalarFraction = modf(x[0][0], scalarWhole);
    outputBuffer[0] = float(scalarFraction);
    outputBuffer[1] = float(scalarWhole);

    int3 vectorWhole;
    TestVector vectorFraction = modf(x[0], vectorWhole);
    for (int i = 0; i < 3; ++i)
    {
        outputBuffer[2 + i] = float(vectorFraction[i]);
        outputBuffer[5 + i] = float(vectorWhole[i]);
    }

    row_major int2x3 rowMajorWhole;
    TestMatrix rowMajorFraction = modf(x, rowMajorWhole);
    column_major int2x3 columnMajorWhole;
    TestMatrix columnMajorFraction = modf(x, columnMajorWhole);
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            int i = row * 3 + col;
            outputBuffer[8 + i] = float(rowMajorFraction[row][col]);
            outputBuffer[14 + i] = float(rowMajorWhole[row][col]);
            outputBuffer[20 + i] = float(columnMajorFraction[row][col]);
            outputBuffer[26 + i] = float(columnMajorWhole[row][col]);
        }
    }

    // We keep the floating-point calls live so every target also checks overload selection.
    TEST_FLOAT scalarFloatWhole;
    TestVector vectorFloatWhole;
    TestMatrix matrixFloatWhole;
    outputBuffer[32] = float(modf(x[0][0], scalarFloatWhole));
    outputBuffer[33] = float(scalarFloatWhole);
    TestVector vectorFloatFraction = modf(x[0], vectorFloatWhole);
    TestMatrix matrixFloatFraction = modf(x, matrixFloatWhole);
    outputBuffer[34] = float(vectorFloatFraction[0]);
    outputBuffer[35] = float(vectorFloatWhole[0]);
    outputBuffer[36] = float(matrixFloatFraction[0][0]);
    outputBuffer[37] = float(matrixFloatWhole[0][0]);

    // Printed zeros do not distinguish their signs. We compare fraction bits for negative zero
    // to verify that integer outputs preserve each target's floating-point `modf` behavior.
    TEST_FLOAT negativeZero = -TEST_FLOAT(inputBuffer[5]);
    scalarFraction = modf(negativeZero, scalarWhole);
    TEST_FLOAT scalarFloatFraction = modf(negativeZero, scalarFloatWhole);
    outputBuffer[38] = float(asuint(float(scalarFraction)) == asuint(float(scalarFloatFraction)));
    vectorFraction = modf(TestVector(negativeZero), vectorWhole);
    vectorFloatFraction = modf(TestVector(negativeZero), vectorFloatWhole);
    outputBuffer[39] = float(
        asuint(float(vectorFraction[0])) == asuint(float(vectorFloatFraction[0])));
    rowMajorFraction = modf(TestMatrix(negativeZero), rowMajorWhole);
    matrixFloatFraction = modf(TestMatrix(negativeZero), matrixFloatWhole);
    outputBuffer[40] = float(
        asuint(float(rowMajorFraction[0][0])) == asuint(float(matrixFloatFraction[0][0])));
}
