//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -render-feature double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -output-using-type

// DXC narrows scalar `double` `modf` to `float` and crashes on double matrices, so we check
// double-precision results on the targets whose native operations support that precision.
// We form 16777217.5 in double precision from buffer inputs to prevent constant folding.
// Converting to `float` before splitting would lose both the fraction and the low integer bit.
//TEST_INPUT:ubuffer(data=[16777216.0 1.5], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=wholeParts
RWStructuredBuffer<int> wholeParts;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=fractionalParts
RWStructuredBuffer<float> fractionalParts;

// BUF: type: int32_t
// BUF-NEXT: 16777217
// BUF-NEXT: -16777217
// BUF-NEXT: 16777217
// BUF-NEXT: -16777217
// BUF-NEXT: 16777217
// BUF-NEXT: -16777217
// BUF-NEXT: 16777217
// BUF-NEXT: -16777217
// BUF: type: float
// BUF-NEXT: 0.5
// BUF-NEXT: -0.5
// BUF-NEXT: 0.5
// BUF-NEXT: -0.5
// BUF-NEXT: 0.5
// BUF-NEXT: -0.5
// BUF-NEXT: 0.5
// BUF-NEXT: -0.5

/// Write both parts of double-precision values that cannot be represented exactly as `float`.
[numthreads(1, 1, 1)]
void computeMain()
{
    double x = double(inputBuffer[0]) + double(inputBuffer[1]);
    int scalarWhole;
    double scalarFraction = modf(x, scalarWhole);
    wholeParts[0] = scalarWhole;
    fractionalParts[0] = float(scalarFraction);
    scalarFraction = modf(-x, scalarWhole);
    wholeParts[1] = scalarWhole;
    fractionalParts[1] = float(scalarFraction);

    int2 vectorWhole;
    double2 vectorFraction = modf(double2(x, -x), vectorWhole);
    for (int i = 0; i < 2; ++i)
    {
        wholeParts[2 + i] = vectorWhole[i];
        fractionalParts[2 + i] = float(vectorFraction[i]);
    }

    int2x2 matrixWhole;
    double2x2 matrixFraction = modf(double2x2(x, -x, x, -x), matrixWhole);
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 2; ++col)
        {
            int i = 4 + row * 2 + col;
            wholeParts[i] = matrixWhole[row][col];
            fractionalParts[i] = float(matrixFraction[row][col]);
        }
    }
}
