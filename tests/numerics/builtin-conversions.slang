// Verify representative explicit conversions between generic builtin scalar element types. The
// matrix case also type-checks conversion of a non-default matrix layout.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o builtin-conversions.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o builtin-conversions.spv -experimental-feature
//TEST:SIMPLE: -target wgsl -entry computeMain -stage compute -o builtin-conversions.wgsl -experimental-feature
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o builtin-conversions.metal -experimental-feature
//TEST:SIMPLE: -target cuda -entry computeMain -stage compute -o builtin-conversions.cuda.cpp -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: -2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: 1
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

TDestination convertScalar<
    TDestination : IBuiltinScalarTypeDispatchMarker,
    TSource : IBuiltinScalarTypeDispatchMarker>(TSource value)
{
    return convertBuiltinScalar<TDestination>(value);
}

vector<TDestination, N> convertVector<
    TDestination : IBuiltinScalarTypeDispatchMarker,
    TSource : IBuiltinScalarTypeDispatchMarker,
    let N : int>(vector<TSource, N> value)
{
    return convertBuiltinVector<TDestination>(value);
}

matrix<TDestination, R, C, L> convertMatrix<
    TDestination : IBuiltinScalarTypeDispatchMarker,
    TSource : IBuiltinScalarTypeDispatchMarker,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(matrix<TSource, R, C, L> value)
{
    return convertBuiltinMatrix<TDestination>(value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int scalarFromFloat = convertScalar<int>(3.75);
    float scalarFromBool = convertScalar<float>(true);
    bool scalarFromInt = convertScalar<bool>(2);
    bool scalarBoolFromFractionalFloat = convertScalar<bool>(0.5);
    bool constructorBoolFromFractionalFloat = bool(0.5);

    int3 vectorFromFloat = convertVector<int>(float3(1.0, -2.0, 3.0));
    vector<bool, 3> vectorFromInt = convertVector<bool>(int3(0, 2, -1));

    matrix<float, 2, 2, MatrixLayoutMode.RowMajor> sourceMatrix =
        matrix<float, 2, 2, MatrixLayoutMode.RowMajor>(4.0, 5.0, 6.0, 7.0);
    matrix<uint, 2, 2, MatrixLayoutMode.RowMajor> matrixFromFloat =
        convertMatrix<uint>(sourceMatrix);

    outputBuffer[0] = scalarFromFloat;
    outputBuffer[1] = int(scalarFromBool);
    outputBuffer[2] = int(scalarFromInt);
    outputBuffer[3] = vectorFromFloat.x;
    outputBuffer[4] = vectorFromFloat.y;
    outputBuffer[5] = vectorFromFloat.z;
    outputBuffer[6] = int(matrixFromFloat[0][0]);
    outputBuffer[7] = int(matrixFromFloat[0][1]);
    outputBuffer[8] = int(matrixFromFloat[1][0]);
    outputBuffer[9] = int(matrixFromFloat[1][1]);
    outputBuffer[10] = int(vectorFromInt.x);
    outputBuffer[11] = int(vectorFromInt.y);
    outputBuffer[12] = int(vectorFromInt.z);
    outputBuffer[15] = int(scalarBoolFromFractionalFloat);
    outputBuffer[16] = int(constructorBoolFromFractionalFloat);

    // Exercise half/double conversions only on targets that represent both types.
    __target_switch
    {
    case cpp:
    case cuda:
        half halfFromDouble = convertScalar<half>(double(8.5));
        double doubleFromHalf = convertScalar<double>(half(9.5));
        outputBuffer[13] = int(halfFromDouble);
        outputBuffer[14] = int(doubleFromHalf);
    default:
        outputBuffer[13] = 0;
        outputBuffer[14] = 0;
    }
}
