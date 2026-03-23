// Debug: dump forward output values for VK tiled layout test.
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0 -emit-spirv-directly

#pragma warning(disable: 41017)
import slang.neural;
import common;

typealias ElementType = float;

//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)

#if TEST_POINTER
    uniform ElementType* parameters;
    typealias Address = PointerAddress<ElementType>;
#else
    uniform RWStructuredBuffer<ElementType>.Handle parameters;
    typealias Address = BindlessAddress<ElementType>;
#endif

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

// Dump: output[0..1] = forward result, output[2..9] = weight tile row 0, output[10..17] = weight tile row 1
// TEST_INPUT: ubuffer(data=[0], stride=4, count=32):out, name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // Setup weights
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

    // Dump weight buffer (first 16 elements = row 0 of tile)
    if (tid == 0)
    {
        for (int i = 0; i < 16; i++)
            resultBuffer[2 + i] = float(parameters[i]);
    }

    // Forward without bias
    typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);
    Address weightAddress = Address(parameters);

    let outputVec = input.linearTransform<Address, OptimalLayout, OutVectorType>(weightAddress);

    if (tid == 0)
    {
        resultBuffer[0] = float(outputVec[0]);
        resultBuffer[1] = float(outputVec[1]);
    }

    // BUFFER: {{.*}}
}
