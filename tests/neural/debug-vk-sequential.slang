// Debug: check state after BackwardWithoutBias
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_POINTER=0 -emit-spirv-directly

#pragma warning(disable: 41017)
import slang.neural;
import common;

typealias ElementType = float;

//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;
//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)
//TEST_INPUT: set dParameters = ubuffer(stride=4, count=258)
uniform RWStructuredBuffer<ElementType>.Handle parameters;
uniform RWStructuredBuffer<ElementType>.Handle dParameters;
typealias Address = BindlessAddress<ElementType>;

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;
typealias InVec = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
typealias OutVec = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

// [0..3]=dInput, [4..7]=dW row0, [8..11]=dW row1, [12..15]=params row0, [16..19]=params row1+bias
// TEST_INPUT: ubuffer(data=[0], stride=4, count=32):out, name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

[Differentiable]
OutVec MatVecMul(InVec input, Address address) {
    return input.linearTransform<Address, OptimalLayout, OutVec>(address);
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid == 0)
    {
        for (int i = 0; i < 258; i++) parameters[i] = ElementType(0.0);
        for (int i = 0; i < 258; i++) dParameters[i] = ElementType(0.0);
        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(0, col)] = ElementType(parametersFloat[col]);
        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(1, col)] = ElementType(parametersFloat[InputSize + col]);
        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
    AllMemoryBarrierWithGroupSync();

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);
    Address weightAddress = Address(parameters);
    Address dWeightAddress = Address(dParameters);

    // Run BackwardWithoutBias
    let outputVec = MatVecMul(input, weightAddress);
    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    let dRes = OutVec(1.0);
    var dPair = diffPair(input);
    bwd_diff(MatVecMul)(dPair, weightDiffPair, dRes);

    // Dump state after backward
    if (tid == 0)
    {
        // dInput: expect [6, 8, 10, 12]
        resultBuffer[16] = float(dPair.d[0]);
        resultBuffer[17] = float(dPair.d[1]);
        resultBuffer[18] = float(dPair.d[2]);
        resultBuffer[19] = float(dPair.d[3]);

        // dW [0..7]: expect [32,64,96,128, 32,64,96,128]
        resultBuffer[0] = float(dParameters[tiledDWeightOffset(0, 0)]);
        resultBuffer[1] = float(dParameters[tiledDWeightOffset(0, 1)]);
        resultBuffer[2] = float(dParameters[tiledDWeightOffset(0, 2)]);
        resultBuffer[3] = float(dParameters[tiledDWeightOffset(0, 3)]);
        resultBuffer[4] = float(dParameters[tiledDWeightOffset(1, 0)]);
        resultBuffer[5] = float(dParameters[tiledDWeightOffset(1, 1)]);
        resultBuffer[6] = float(dParameters[tiledDWeightOffset(1, 2)]);
        resultBuffer[7] = float(dParameters[tiledDWeightOffset(1, 3)]);

        // params after backward [8..15]: expect [1,2,3,4, 5,6,7,8] (should be UNCHANGED)
        resultBuffer[8]  = float(parameters[tiledWeightOffset(0, 0)]);
        resultBuffer[9]  = float(parameters[tiledWeightOffset(0, 1)]);
        resultBuffer[10] = float(parameters[tiledWeightOffset(0, 2)]);
        resultBuffer[11] = float(parameters[tiledWeightOffset(0, 3)]);
        resultBuffer[12] = float(parameters[tiledWeightOffset(1, 0)]);
        resultBuffer[13] = float(parameters[tiledWeightOffset(1, 1)]);
        resultBuffer[14] = float(parameters[tiledWeightOffset(1, 2)]);
        resultBuffer[15] = float(parameters[tiledWeightOffset(1, 3)]);
    }

    // BUFFER: {{.*}}
}
