// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

import neural;
#pragma warning(disable: 41017)

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT:ubuffer(stride=4, count=16):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

bool verify<int InputSize, int OutputSize, ExecutionMode Mode>(int actualSize)
{
    // In this test, the CoopMat shape is always 16x16x16.
    static const int coopMatShape = 16;
    static const int alignedK = ((InputSize + coopMatShape - 1) / coopMatShape) * coopMatShape;
    static const int alignedM = ((OutputSize + coopMatShape - 1) / coopMatShape) * coopMatShape;
    static const int aliggnedN = SubgroupSize;
    static const int subgroupCount = BatchSize / SubgroupSize;

    static const int tileASize = Mode == ExecutionMode.Inference ?
                alignedM * coopMatShape * sizeof(half) :               // Forward,  A is M x K, tile A is M x 16
                alignedK * coopMatShape * sizeof(half);                // Backward, A is K x M, tile A is K x 16

    // in backward, B is M x N, and N x K, tile B is 16 x max(N, K)
    static const int maxTileBSizeInBackward = (aliggnedN > alignedK ? aliggnedN: alignedK) * coopMatShape;

    static const int tileBSize = Mode == ExecutionMode.Inference ?
                coopMatShape * aliggnedN * sizeof(half) :               // forward,  B is K x N, tile B is 16 x N
                maxTileBSizeInBackward * sizeof(half);                  // backward, B is M x N, tile B is 16 x max(N, K)

    static const int tileCSize = tileBSize * sizeof(DType) / sizeof(half);

    static const int maxTileBCSize = tileBSize > tileCSize ? tileBSize : tileCSize;
    static const int expectedSize = tileASize + maxTileBCSize * subgroupCount;

    return actualSize == expectedSize;
}

void test<int InputSize, int OutputSize, ExecutionMode Mode>(int resIndex)
{
    int size = 0;
    __target_switch
    {
    case cuda:
        size = SharedMemorySize<DType, TargetEnum.CUDA, Mode, SubgroupSize, BatchSize/SubgroupSize, InputSize, OutputSize>.SharedMemSizeInBytes;
        break;
    case spirv:
        size = SharedMemorySize<DType, TargetEnum.SPIR_V, Mode, SubgroupSize, BatchSize/SubgroupSize, InputSize, OutputSize>.SharedMemSizeInBytes;
        break;
    }

    bool res = verify<InputSize, OutputSize, Mode>(size);
    outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid: SV_DispatchThreadID)
{
    // test<5, 9, ExecutionMode.Inference>(0);
    test<5, 9, ExecutionMode.Training>(1);

    // test<17, 23, ExecutionMode.Inference>(2);
    // test<17, 23, ExecutionMode.Training>(3);

    // test<34, 47, ExecutionMode.Inference>(4);
    // test<34, 47, ExecutionMode.Training>(5);

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
