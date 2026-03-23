// Debug: dump dWeight and dBias from BackwardWithBias on VK.
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0 -emit-spirv-directly

#pragma warning(disable: 41017)
import slang.neural;
import common;

typealias ElementType = float;

//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)
//TEST_INPUT: set dParameters = ubuffer(stride=4, count=258)

#if TEST_POINTER
    uniform ElementType* parameters;
    uniform ElementType* dParameters;
    typealias Address = PointerAddress<ElementType>;
#else
    uniform RWStructuredBuffer<ElementType>.Handle parameters;
    uniform RWStructuredBuffer<ElementType>.Handle dParameters;
    typealias Address = BindlessAddress<ElementType>;
#endif

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

// Dump: [0]=fwd0, [1]=fwd1, [2]=dIn0..dIn3, [6..13]=dW row0+row1, [14]=dBias0, [15]=dBias1
// TEST_INPUT: ubuffer(data=[0], stride=4, count=32):out, name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

[Differentiable]
WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>
MatVecMulAdd(
    WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1> input,
    Address weightAddress,
    Address biasAddress)
{
    typealias OutVec = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;
    return input.linearTransform<Address, OptimalLayout, OutVec>(weightAddress, biasAddress);
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // Setup weights
    if (tid == 0)
    {
        for (int i = 0; i < 258; i++)
            parameters[i] = ElementType(0.0);
        for (int i = 0; i < 258; i++)
            dParameters[i] = ElementType(0.0);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(0, col)] = ElementType(parametersFloat[col]);
        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(1, col)] = ElementType(parametersFloat[InputSize + col]);
        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
    AllMemoryBarrierWithGroupSync();

    typealias InVec = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVec = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);

    Address weightAddress = Address(parameters);
    Address biasAddress = weightAddress.getOffset(256);
    Address dWeightAddress = Address(dParameters);
    Address dBiasAddress = dWeightAddress.getOffset(256);

    let outputVec = MatVecMulAdd(input, weightAddress, biasAddress);

    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    var biasDiffPair = DifferentialPtrPair<Address>(biasAddress, dBiasAddress);
    let dOutput = OutVec(1.0);
    var dPair = diffPair(input);

    bwd_diff(MatVecMulAdd)(dPair, weightDiffPair, biasDiffPair, dOutput);

    if (tid == 0)
    {
        resultBuffer[0] = float(outputVec[0]);
        resultBuffer[1] = float(outputVec[1]);
        resultBuffer[2] = float(dPair.d[0]);
        resultBuffer[3] = float(dPair.d[1]);
        resultBuffer[4] = float(dPair.d[2]);
        resultBuffer[5] = float(dPair.d[3]);

        resultBuffer[6] = float(dParameters[tiledDWeightOffset(0, 0)]);
        resultBuffer[7] = float(dParameters[tiledDWeightOffset(0, 1)]);
        resultBuffer[8] = float(dParameters[tiledDWeightOffset(0, 2)]);
        resultBuffer[9] = float(dParameters[tiledDWeightOffset(0, 3)]);
        resultBuffer[10] = float(dParameters[tiledDWeightOffset(1, 0)]);
        resultBuffer[11] = float(dParameters[tiledDWeightOffset(1, 1)]);
        resultBuffer[12] = float(dParameters[tiledDWeightOffset(1, 2)]);
        resultBuffer[13] = float(dParameters[tiledDWeightOffset(1, 3)]);

        resultBuffer[14] = float(dParameters[256]);
        resultBuffer[15] = float(dParameters[257]);
    }

    // BUFFER: {{.*}}
}
