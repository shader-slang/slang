// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly

import slang.neural;

typealias ElementType = float;

//TEST_INPUT: ubuffer(data=[0], stride=4, count=1):out,name=testResult
RWStructuredBuffer<uint> testResult;

//TEST_INPUT: ubuffer(data=[0], stride=4, count=256):out,name=debugBuffer
RWStructuredBuffer<float> debugBuffer;

// Weight matrix: 2x4 [1 2 3 4; 5 6 7 8]
//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set parameters = ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
uniform RWStructuredBuffer<ElementType>.Handle parameters;

typealias Address = BindlessAddress<ElementType>;

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, workgroupCount>;
typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, workgroupCount>;

[Differentiable]
OutVectorType MatVecMul(InVectorType input, Address address)
{
    return input.linearTransform<Address, LinearLayout, OutVectorType>(address);
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid == 0)
    {
        for (int i = 0; i < 8; i++)
            parameters[i] = ElementType(parametersFloat[i]);
    }
    GroupMemoryBarrierWithWaveSync();

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);
    Address weightAddress = Address(parameters);

    let outputVec = MatVecMul(input, weightAddress);

    debugBuffer[tid * 2 + 0] = float(outputVec[0]);
    debugBuffer[tid * 2 + 1] = float(outputVec[1]);

    bool isPassed = (outputVec[0] == 30.0 && outputVec[1] == 70.0);
    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
        testResult[0] = isPassed ? 1 : 0;

    // BUFFER: 1
}
