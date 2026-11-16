// Verify that selection uses the associated mask through the base shaped-type contract, including
// custom storage, explicit matrix layouts, and floating-point components that arithmetic blending
// would change.
// The HLSL case uses Slang 202c so overload selection cannot rely on the deprecated tie-breaker
// that compares generic parameter counts.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -std 202c -o numerics-select.hlsl
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -experimental-feature -o numerics-select.spv

// CHECK: type: int32_t
// CHECK-COUNT-14: {{^}}1{{$}}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
//TEST_INPUT:ubuffer(data=[0x7fc00001 0x80000000 0x7fc00002 0], stride=4):name=inputBits

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;
StructuredBuffer<uint> inputBits;

T chooseComponents<T : INumericShapedType>(T.Mask condition, T ifTrue, T ifFalse)
{
    return select(condition, ifTrue, ifFalse);
}

T positiveMagnitude<T : IReal>(T value)
{
    return select(value >= T(0.0), value, -value);
}

// A `CoordinatePair` has two logical scalar components stored in a struct, while selection uses a
// builtin Boolean vector. Its representation cannot use a native vector select.
struct CoordinatePair : INumericShapedType
{
    typealias Scalar = float;
    typealias Mask = bool2;

    float x;
    float y;

    static This fromScalar(Scalar value)
    {
        return {value, value};
    }

    static This selectComponents(Mask condition, This ifTrue, This ifFalse)
    {
        return {
            condition.x ? ifTrue.x : ifFalse.x,
            condition.y ? ifTrue.y : ifFalse.y};
    }
}

// We check each logical row so the same comparison works for every matrix layout.
bool matricesEqual<T : __BuiltinArithmeticType, let R : int, let C : int, let L : MatrixLayoutMode>(
    matrix<T, R, C, L> actual,
    matrix<T, R, C, L> expected)
{
    for (int row = 0; row < R; ++row)
    {
        if (!(actual[row] == expected[row]).all())
            return false;
    }
    return true;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Both mask values must select the corresponding whole scalar value.
    outputBuffer[0] = int(chooseComponents(true, 3.0, 7.0) == 3.0);
    outputBuffer[1] = int(chooseComponents(false, -4, 9) == 9);
    outputBuffer[2] = int(chooseComponents(true, 11u, 22u) == 11u);

    // Mixed masks distinguish component-wise selection from a whole-value decision.
    let mask = bool3(true, false, true);
    outputBuffer[3] = int((chooseComponents(mask, float3(1, 2, 3), float3(4, 5, 6)) ==
        float3(1, 5, 3)).all());
    outputBuffer[4] = int((chooseComponents(mask, int3(-1, -2, -3), int3(4, 5, 6)) ==
        int3(-1, 5, -3)).all());
    outputBuffer[5] = int((chooseComponents(mask, uint3(1, 2, 3), uint3(4, 5, 6)) ==
        uint3(1, 5, 3)).all());
    outputBuffer[6] = int((positiveMagnitude(float3(-2, 0, 3)) == float3(2, 0, 3)).all());

    // The base contract also covers shape-only integer matrices and both explicit layouts.
    typealias RowMatrix = matrix<float, 2, 3, MatrixLayoutMode.RowMajor>;
    let matrixMask = matrix<bool, 2, 3, MatrixLayoutMode.RowMajor>(
        true, false, true, false, true, false);
    let rowResult = chooseComponents(matrixMask, RowMatrix(1, 2, 3, 4, 5, 6), RowMatrix(10));
    outputBuffer[7] = int(matricesEqual(rowResult, RowMatrix(1, 10, 3, 10, 5, 10)));

    typealias ColumnMatrix = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;
    let columnResult = chooseComponents(
        matrix<bool, 2, 3, MatrixLayoutMode.ColumnMajor>(matrixMask),
        ColumnMatrix(1, 2, 3, 4, 5, 6), ColumnMatrix(10));
    outputBuffer[8] = int(matricesEqual(columnResult, ColumnMatrix(1, 10, 3, 10, 5, 10)));
    let integerResult = chooseComponents(
        bool2x2(true, false, false, true), int2x2(1, 2, 3, 4), int2x2(9));
    outputBuffer[9] = int(matricesEqual(integerResult, int2x2(1, 9, 9, 4)));

    // Concrete builtin calls must remain compatible with the new generic fallback.
    outputBuffer[10] = int((select(mask, float3(1, 2, 3), float3(4, 5, 6)) ==
        float3(1, 5, 3)).all());
    let pairResult = chooseComponents(
        bool2(false, true), CoordinatePair(1, 2), CoordinatePair(3, 4));
    outputBuffer[11] = int(pairResult.x == 3.0 && pairResult.y == 2.0);

    // Buffer-loaded bits keep NaN payloads and zero signs observable after specialization. Each
    // selected component must preserve its bits even when the other argument contains a NaN.
    let expectedBits = uint4(inputBits[0], inputBits[1], inputBits[2], inputBits[3]);
    let ifTrue = asfloat(uint4(inputBits[0], inputBits[1], 0, inputBits[2]));
    let ifFalse = asfloat(uint4(0, 0, inputBits[2], inputBits[3]));
    let bitMask = bool4(inputBits[0] != 0, inputBits[1] != 0, inputBits[2] == 0, inputBits[3] != 0);
    let selected = chooseComponents(bitMask, ifTrue, ifFalse);
    outputBuffer[12] = int((asuint(selected) == expectedBits).all());
    let selectedMatrix = chooseComponents(
        bool2x2(bitMask.xy, bitMask.zw),
        float2x2(ifTrue.xy, ifTrue.zw), float2x2(ifFalse.xy, ifFalse.zw));
    outputBuffer[13] = int((asuint(selectedMatrix[0]) == expectedBits.xy).all() &&
        (asuint(selectedMatrix[1]) == expectedBits.zw).all());
}
