// Test automatic differentiation through the subscript getter and setter of both vector
// implementations.
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -xslang -experimental-feature -output-using-type -capability cuda_sm_7_0

import slang.neural;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

static const int VectorSize = 3;
static const int SubgroupSize = 32;

typealias TestShMemSize = SharedMemorySize<
    float,
    TargetEnum.CUDA,
    ExecutionMode.Training,
    SubgroupSize,
    1,
    VectorSize,
    VectorSize>;
typealias InlineTestVector = InlineVector<float, VectorSize>;
typealias WaveTangledTestVector =
    WaveTangledVector<float, TestShMemSize, VectorSize, SubgroupSize, 1>;

[Differentiable]
float readElement<Vector>(Vector value)
    where Vector : IVector<float>
{
    return value[1];
}

[Differentiable]
Vector writeElement<Vector>(Vector value, float newValue)
    where Vector : IVector<float>
{
    value[1] = newValue;
    return value;
}

bool[4] testVectorAutodiffSubscript<Vector>(
    DifferentialPair<Vector> value,
    Vector.Differential zero,
    Vector.Differential dOutput)
    where Vector : IVector<float>
{
    bool results[4];

    let getterForwardResult = fwd_diff(readElement<Vector>)(value);
    results[0] = getterForwardResult.p == 2.0 && getterForwardResult.d == 3.0;

    var getterBackwardValue = DifferentialPair<Vector>(value.p, zero);
    bwd_diff(readElement<Vector>)(getterBackwardValue, 1.0);
    let getterBackwardDiff = getterBackwardValue.d;
    results[1] = getterBackwardDiff[0] == 0.0 &&
                 getterBackwardDiff[1] == 1.0 &&
                 getterBackwardDiff[2] == 0.0;

    let setterForwardResult =
        fwd_diff(writeElement<Vector>)(value, diffPair(4.0, 7.0));
    let setterForwardDiff = setterForwardResult.d;
    results[2] = setterForwardResult.p[0] == 1.0 &&
                 setterForwardResult.p[1] == 4.0 &&
                 setterForwardResult.p[2] == 3.0 &&
                 setterForwardDiff[0] == 2.0 &&
                 setterForwardDiff[1] == 7.0 &&
                 setterForwardDiff[2] == 5.0;

    var setterBackwardValue = DifferentialPair<Vector>(value.p, zero);
    var newValue = diffPair(4.0);
    bwd_diff(writeElement<Vector>)(setterBackwardValue, newValue, dOutput);
    let setterBackwardDiff = setterBackwardValue.d;
    results[3] = setterBackwardDiff[0] == 2.0 &&
                 setterBackwardDiff[1] == 0.0 &&
                 setterBackwardDiff[2] == 5.0 &&
                 newValue.d == 3.0;

    return results;
}

[shader("compute")]
[numthreads(SubgroupSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    float[VectorSize] data = {1.0, 2.0, 3.0};
    float[VectorSize] dData = {2.0, 3.0, 5.0};
    float[VectorSize] dOutputData = {2.0, 3.0, 5.0};

    let inlineResults = testVectorAutodiffSubscript<InlineTestVector>(
        diffPair(InlineTestVector(data), InlineTestVector(dData)),
        InlineTestVector(),
        InlineTestVector(dOutputData));
    let waveTangledResults =
        testVectorAutodiffSubscript<WaveTangledTestVector>(
            diffPair(WaveTangledTestVector(data), WaveTangledTestVector(dData)),
            WaveTangledTestVector(),
            WaveTangledTestVector(dOutputData));

    bool results[8];
    for (int i = 0; i < 4; i++)
    {
        results[i] = WaveActiveAllTrue(inlineResults[i]);
        results[i + 4] = WaveActiveAllTrue(waveTangledResults[i]);
    }

    if (tid == 0)
    {
        for (int i = 0; i < 8; i++)
            testResult[i] = results[i];
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
