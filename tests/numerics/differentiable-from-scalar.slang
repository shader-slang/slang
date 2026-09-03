// Verify that a varying logical scalar remains differentiable when it is repeated across a
// builtin or user-defined shaped numeric value.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -experimental-feature

import slang.numerics.differentiable;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
T repeatScalar<T : IDifferentiableNumericShapedType>(T.Scalar value)
{
    return T.fromScalar(value);
}

[Differentiable]
float3 repeatVectorScalar(float value)
{
    return repeatScalar<float3>(value);
}

[Differentiable]
float2x2 repeatMatrixScalar(float value)
{
    return repeatScalar<float2x2>(value);
}

struct CustomPair : IDifferentiableNumericShapedType
{
    typealias Scalar = float;
    typealias Mask = bool;

    float first;
    float second;

    [Differentiable]
    static This fromScalar(Scalar value)
    {
        return {value, value};
    }
}

[Differentiable]
CustomPair repeatCustomScalar(float value)
{
    return repeatScalar<CustomPair>(value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let vectorForward = fwd_diff(repeatVectorScalar)(diffPair(2.0, 3.0));
    outputBuffer[0] = vectorForward.d.x + vectorForward.d.y + vectorForward.d.z; // CHECK: 9.0

    var vectorReverse = diffPair(2.0, 0.0);
    bwd_diff(repeatVectorScalar)(vectorReverse, float3(1.0, 2.0, 3.0));
    outputBuffer[1] = vectorReverse.d; // CHECK: 6.0

    let matrixForward = fwd_diff(repeatMatrixScalar)(diffPair(2.0, 3.0));
    outputBuffer[2] = matrixForward.d[0][0] + matrixForward.d[0][1] +
        matrixForward.d[1][0] + matrixForward.d[1][1]; // CHECK: 12.0

    var matrixReverse = diffPair(2.0, 0.0);
    bwd_diff(repeatMatrixScalar)(matrixReverse, float2x2(1.0, 2.0, 3.0, 4.0));
    outputBuffer[3] = matrixReverse.d; // CHECK: 10.0

    let customForward = fwd_diff(repeatCustomScalar)(diffPair(2.0, 3.0));
    outputBuffer[4] = customForward.d.first + customForward.d.second; // CHECK: 6.0

    var customReverse = diffPair(2.0, 0.0);
    bwd_diff(repeatCustomScalar)(customReverse, CustomPair(4.0, 5.0));
    outputBuffer[5] = customReverse.d; // CHECK: 9.0
}
