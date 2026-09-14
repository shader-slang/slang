// Verify generation and mapping for arrays, vectors, and matrices with function values and
// capturing lambdas.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.functional;

RWStructuredBuffer<int> outputBuffer;

float addHalf(int value)
{
    return float(value) + 0.5;
}

int quantize(float value)
{
    return int(value * 2.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int arrayBase = 2;
    Array<int, 4> generatedArray = Array<int, 4>((int index) => arrayBase + index);
    Array<int, 4> factoryArray = Array<int, 4>.generate((int index) => index * index);
    let functionMappedArray = generatedArray.map(addHalf);
    int arrayOffset = 10;
    let closureMappedArray =
        generatedArray.map((int value) => float(value + arrayOffset) + 0.25);

    outputBuffer[0] = int(generatedArray[0] == 2 && generatedArray[3] == 5);
    outputBuffer[1] = int(factoryArray[1] == 1 && factoryArray[3] == 9);
    outputBuffer[2] = int(functionMappedArray[0] == 2.5 && functionMappedArray[3] == 5.5);
    outputBuffer[3] = int(closureMappedArray[0] == 12.25 && closureMappedArray[3] == 15.25);

    float4 generatedVector = float4((int index) => float(index) + 0.5);
    int4 factoryVector = int4.generate((int index) => index * 3);
    let functionMappedVector = generatedVector.map(quantize);
    float vectorScale = 2.0;
    let closureMappedVector = generatedVector.map((float value) => value * vectorScale);

    outputBuffer[4] = int(generatedVector.x == 0.5 && generatedVector.w == 3.5);
    outputBuffer[5] = int(factoryVector.x == 0 && factoryVector.w == 9);
    outputBuffer[6] = int(functionMappedVector.x == 1 && functionMappedVector.w == 7);
    outputBuffer[7] = int(closureMappedVector.x == 1.0 && closureMappedVector.w == 7.0);

    int matrixBase = 20;
    matrix<int, 2, 3, kRowMajorMatrixLayout> generatedMatrix =
        matrix<int, 2, 3, kRowMajorMatrixLayout>(
            (int row, int column) => matrixBase + row * 10 + column);
    matrix<int, 2, 3, kRowMajorMatrixLayout> factoryMatrix =
        matrix<int, 2, 3, kRowMajorMatrixLayout>.generate(
            (int row, int column) => row * 10 + column);
    matrix<float, 2, 3, kRowMajorMatrixLayout> functionMappedMatrix =
        generatedMatrix.map(addHalf);
    int matrixOffset = 5;
    matrix<float, 2, 3, kRowMajorMatrixLayout> closureMappedMatrix =
        generatedMatrix.map((int value) => float(value + matrixOffset) + 0.25);

    outputBuffer[8] = int(generatedMatrix[0][0] == 20 && generatedMatrix[1][2] == 32);
    outputBuffer[9] = int(factoryMatrix[0][0] == 0 && factoryMatrix[1][2] == 12);
    outputBuffer[10] =
        int(functionMappedMatrix[0][0] == 20.5 && functionMappedMatrix[1][2] == 32.5);
    outputBuffer[11] =
        int(closureMappedMatrix[0][0] == 25.25 && closureMappedMatrix[1][2] == 37.25);
}
