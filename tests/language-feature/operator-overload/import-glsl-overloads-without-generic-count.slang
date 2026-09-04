//TEST:SIMPLE: -std 2026 -warnings-as-errors 40021 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -target spirv-asm -entry computeMain -stage compute

// Slang 202c does not use generic parameter count to rank overloads. The surviving GLSL
// declarations must therefore beat the core-module operators through OverloadRank alone.
import glsl;

RWStructuredBuffer<float> outputBuffer;

// These formerly overlapping operations only need to type-check unambiguously in both language
// modes; code generation is covered separately by `computeMain`.
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

    half2x2 halfLeft = half2x2(1.0h, 2.0h, 3.0h, 4.0h);
    half2x2 halfIdentity = half2x2(1.0h, 0.0h, 0.0h, 1.0h);
    half2x2 halfProduct = halfLeft * halfIdentity;

    double2x2 doubleLeft = double2x2(1.0, 2.0, 3.0, 4.0);
    double2x2 doubleRight = double2x2(5.0, 6.0, 7.0, 8.0);
    double2x2 doubleProduct = doubleLeft * doubleRight;

    float4 vectorValue = float4(1.0, 2.0, 3.0, 4.0);
    bool vectorsEqual = vectorValue == vectorValue;
    outputBuffer[0] = vectorsEqual ? product[0][1] : 0.0;
    outputBuffer[1] = halfProduct[0][1];
    outputBuffer[2] = float(doubleProduct[0][1]);
}

// CHECK-COUNT-3: OpMatrixTimesMatrix
