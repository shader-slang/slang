//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct NumericValues
{
    float3 vectorValue;
    float2x3 matrixValue;
}

[noinline]
float4 evaluateVectorArithmetic(float4 left, float4 right)
{
    return (left + right) * right / right - left;
}

[noinline]
int4 evaluateIntegerArithmetic(int4 left, int4 right)
{
    return (left + right) * right / right - left;
}

[noinline]
float4 evaluateSpecialFunctions(float4 value)
{
    return sin(value) + cos(value) + tan(value) + sqrt(value) + pow(value, value);
}

[noinline]
bool4 evaluateComparisons(float4 left, float4 right)
{
    return !(left < right);
}

[noinline]
float2x3 evaluateMatrix(float2x3 left, float2x3 right)
{
    return (left + right) * right / right;
}

[noinline]
int2x2 evaluateIntegerMatrix(int2x2 left, int2x2 right)
{
    return (left + right) * right / right - left;
}

[noinline]
float2x2 evaluateMatrixSpecialFunction(float2x2 value)
{
    return sqrt(value);
}

[noinline]
bool2x3 evaluateMatrixComparison(float2x3 left, float2x3 right)
{
    return !(left < right);
}

[noinline]
float3 returnVector(float3 value)
{
    return value;
}

[noinline]
void updateValues(inout NumericValues values)
{
    values.vectorValue[1] = 42.0;
    values.matrixValue[1][2] = 77.0;
}

[host]
void main()
{
    float4 vectorResult = evaluateVectorArithmetic(
        float4(2.0, 4.0, 6.0, 8.0),
        float4(1.0, 2.0, 3.0, 4.0));
    print(vectorResult[0]);
    print(vectorResult[1]);
    print(vectorResult[2]);
    print(vectorResult[3]);

    int4 integerResult = evaluateIntegerArithmetic(
        int4(-2, 4, -6, 8),
        int4(1, 2, 3, 4));
    print(integerResult[0]);
    print(integerResult[1]);
    print(integerResult[2]);
    print(integerResult[3]);

    float4 specialResult = evaluateSpecialFunctions(float4(1.0));
    print(specialResult[0]);
    print(specialResult[3]);

    bool4 comparisonResult = evaluateComparisons(
        float4(1.0, 4.0, 3.0, 8.0),
        float4(2.0, 3.0, 3.0, 7.0));
    print(comparisonResult[0]);
    print(comparisonResult[1]);
    print(comparisonResult[2]);
    print(comparisonResult[3]);

    float2x3 matrixResult = evaluateMatrix(
        float2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0),
        float2x3(6.0, 5.0, 4.0, 3.0, 2.0, 1.0));
    print(matrixResult[0][0]);
    print(matrixResult[0][2]);
    print(matrixResult[1][0]);
    print(matrixResult[1][2]);

    int2x2 integerMatrixResult = evaluateIntegerMatrix(
        int2x2(-6, 4, -2, 8),
        int2x2(1, 2, 3, 4));
    print(integerMatrixResult[0][0]);
    print(integerMatrixResult[0][1]);
    print(integerMatrixResult[1][0]);
    print(integerMatrixResult[1][1]);

    float2x2 matrixSpecialResult = evaluateMatrixSpecialFunction(float2x2(1.0));
    print(matrixSpecialResult[0][0]);
    print(matrixSpecialResult[1][1]);

    bool2x3 matrixComparisonResult = evaluateMatrixComparison(
        float2x3(1.0, 4.0, 3.0, 8.0, 5.0, 9.0),
        float2x3(2.0, 3.0, 3.0, 7.0, 6.0, 8.0));
    print(matrixComparisonResult[0][0]);
    print(matrixComparisonResult[0][1]);
    print(matrixComparisonResult[1][1]);
    print(matrixComparisonResult[1][2]);

    float4 swizzleSource = float4(10.0, 20.0, 30.0, 40.0);
    float3 swizzled = returnVector(swizzleSource.wyx);
    print(swizzled[0]);
    print(swizzled[1]);
    print(swizzled[2]);

    NumericValues values;
    values.vectorValue = float3(7.0, 8.0, 9.0);
    values.matrixValue = float2x3(10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    updateValues(values);
    print(values.vectorValue[0]);
    print(values.vectorValue[1]);
    print(values.matrixValue[1][1]);
    print(values.matrixValue[1][2]);
}

// CHECK: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 4.939181
// CHECK-NEXT: 4.939181
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: 40.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 42.000000
// CHECK-NEXT: 14.000000
// CHECK-NEXT: 77.000000
