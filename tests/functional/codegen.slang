// Verify that functional operations specialize their callable values before target emission.

//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.hlsl
//TEST:SIMPLE: -target spirv-asm -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.spv-asm
//TEST:SIMPLE: -target glsl -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.glsl
//TEST:SIMPLE: -target wgsl -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.wgsl
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.metal
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.cpp
//TEST:SIMPLE: -target cuda -entry computeMain -stage compute -experimental-feature -validate-ir -o functional.cuda.cpp

import slang.functional;

RWStructuredBuffer<int4> outputBuffer;

int quantize(float value)
{
    return int(value * 2.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int baseValue = 1;
    Array<int, 4> arrayValue = Array<int, 4>((int index) => baseValue + index);
    let mappedArray = arrayValue.map((int value) => value + baseValue);

    float4 vectorValue = float4((int index) => float(index) + 0.5);
    let mappedVector = vectorValue.map(quantize);

    matrix<int, 2, 2> matrixValue =
        matrix<int, 2, 2>((int row, int column) => row * 2 + column);
    let mappedMatrix = matrixValue.map((int value) => value + baseValue);

    outputBuffer[0] = int4(
        mappedArray[0] + mappedMatrix[0][0],
        mappedArray[1] + mappedMatrix[0][1],
        mappedVector[2] + mappedMatrix[1][0],
        mappedVector[3] + mappedMatrix[1][1]);
}
