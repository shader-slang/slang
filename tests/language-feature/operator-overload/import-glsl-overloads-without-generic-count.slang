//TEST:SIMPLE: -std 202c -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -target spirv-asm -entry computeMain -stage compute

// Slang 202c does not use generic parameter count to rank overloads. The surviving GLSL
// declarations must therefore beat the core-module operators through OverloadRank alone.
import glsl;

RWStructuredBuffer<float> outputBuffer;

void typeCheckRemovedOverloads(
    half2x2 halfMatrix,
    double2x2 doubleMatrix,
    matrix<float, 2, 3> rectangularLeft,
    matrix<float, 4, 2> rectangularRight,
    int4 intVector,
    bool4 boolVector)
{
    half2x2 halfProduct = halfMatrix * halfMatrix;
    double2x2 doubleProduct = doubleMatrix * doubleMatrix;
    matrix<float, 4, 3> rectangularProduct = rectangularLeft * rectangularRight;
    bool intsEqual = intVector == intVector;
    bool boolsNotEqual = boolVector != boolVector;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 left = float2x2(1.0, 2.0, 3.0, 4.0);
    float2x2 identity = float2x2(1.0, 0.0, 0.0, 1.0);
    float2x2 product = left * identity;

    float4 vectorValue = float4(1.0, 2.0, 3.0, 4.0);
    bool vectorsEqual = vectorValue == vectorValue;
    outputBuffer[0] = vectorsEqual ? product[0][1] : 0.0;
}

// CHECK: OpMatrixTimesMatrix
