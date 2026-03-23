// Debug CUDA forward test
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0

#pragma warning(disable: 41017)
import slang.neural;
import common;

typealias ElementType = float;

//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)
uniform RWStructuredBuffer<ElementType>.Handle parameters;
typealias Address = BindlessAddress<ElementType>;

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

// [0]=fwd0, [1]=fwd1, [2..17]=weight tile first 16 elements
// TEST_INPUT: ubuffer(data=[0], stride=4, count=32):out, name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid == 0)
    {
        for (int i = 0; i < 258; i++)
            parameters[i] = ElementType(0.0);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(0, col)] = ElementType(parametersFloat[col]);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(1, col)] = ElementType(parametersFloat[InputSize + col]);

        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
    AllMemoryBarrierWithGroupSync();

    // Dump tiledWeightOffset values and actual weight positions
    if (tid == 0)
    {
        // Show where weights were placed
        resultBuffer[2] = float(tiledWeightOffset(0, 0));
        resultBuffer[3] = float(tiledWeightOffset(0, 1));
        resultBuffer[4] = float(tiledWeightOffset(0, 2));
        resultBuffer[5] = float(tiledWeightOffset(0, 3));
        resultBuffer[6] = float(tiledWeightOffset(1, 0));
        resultBuffer[7] = float(tiledWeightOffset(1, 1));
        resultBuffer[8] = float(tiledWeightOffset(1, 2));
        resultBuffer[9] = float(tiledWeightOffset(1, 3));
        // Show what loadWeightRaw expects (raw buffer at positions used by lane 0)
        resultBuffer[10] = float(parameters[0]); // lane0, reg0
        resultBuffer[11] = float(parameters[1]); // lane0, reg1
        resultBuffer[12] = float(parameters[2]); // lane0, reg2
        resultBuffer[13] = float(parameters[3]); // lane0, reg3
        resultBuffer[14] = float(parameters[4]); // lane0, reg4
        resultBuffer[15] = float(parameters[5]); // lane0, reg5
        resultBuffer[16] = float(parameters[6]); // lane0, reg6
        resultBuffer[17] = float(parameters[7]); // lane0, reg7
    }

    // Forward without bias
    typealias InVec = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVec = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);
    Address weightAddress = Address(parameters);

    let outputVec = input.linearTransform<Address, OptimalLayout, OutVec>(weightAddress);

    if (tid == 0)
    {
        resultBuffer[0] = float(outputVec[0]);  // expect 30
        resultBuffer[1] = float(outputVec[1]);  // expect 70
    }

    // BUFFER: {{.*}}
}
