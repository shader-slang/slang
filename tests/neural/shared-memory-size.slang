// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

// Test that given the layer configuration, thread count, data type, subgroup size, and execution mode, the shared memory required is correctly
// calculated during compile time.

// SharedMemorySize<DType, TargetEnum, ExecutionMode, SubgroupSize, SubgroupCount>.OfHiddenN<LayerConfiguration>.Bytes
// is the compile time (or link time) constant that represents the total shared memory needed for launcing the inference kernel or training kernel.

import slang.neural;
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

// Compute the shared memory size to verify the output.
// This verify function uses naive implementation to calculate the shared memory size at run time.
// It just calculate the shared memory size for a single layer, and store the result in the result array.
// Then we will find the maximum value in the result array.
void computeOneLayer<ExecutionMode Mode, T, U, Arr:IRWArray<uint>>(T inputSize, U outputSize, inout uint index, out Arr result)
    where T == uint
    where U == uint
{
    const int coopMatShape = 16;
    const int alignedK = ((inputSize + coopMatShape - 1) / coopMatShape) * coopMatShape;
    const int alignedM = ((outputSize + coopMatShape - 1) / coopMatShape) * coopMatShape;
    const int alignedN = SubgroupSize;
    const int subgroupCount = BatchSize / SubgroupSize;
    const bool isTraining = Mode == ExecutionMode.Training;

    // Term 1: BytesForMMA — A staging + per-warp C staging
    const int tileASize = !isTraining ?
                alignedM * coopMatShape * sizeof(half) :
                max(alignedM, alignedK) * coopMatShape * sizeof(half);

    const int tileBSize = !isTraining ?
                coopMatShape * alignedN * sizeof(half) :
                coopMatShape * (max(alignedN, alignedK)) * sizeof(half);

    const int tileCSize = tileBSize * sizeof(DType) / sizeof(half);
    const int bytesForMMA = tileASize + tileCSize * subgroupCount;

    // Term 2: BytesForCrossWarpReduction — butterfly outer product reduction
    const int nTileRows = (int(outputSize) + coopMatShape - 1) / coopMatShape;
    const int nTileCols = (int(inputSize) + coopMatShape - 1) / coopMatShape;
    const int elementsPerLane = (coopMatShape * coopMatShape) / SubgroupSize;
    const int regsPerLane = (elementsPerLane * sizeof(DType) + 3) / 4;
    const int tileStride = (regsPerLane * SubgroupSize * 4 + sizeof(uint4) - 1) / sizeof(uint4);
    const int bytesForCrossWarpReduction = !isTraining ? 0 :
        max(subgroupCount / 2, 1) * nTileRows * nTileCols * tileStride * sizeof(uint4);

    // Term 3: BytesForOuterProductStore — per-row MatC tile store
    const int bytesForOuterProductStore = !isTraining ? 0 :
        nTileCols * coopMatShape * coopMatShape * sizeof(DType);

    // Term 4: BytesForSumReduceRows — bias reduce
    const int heightInVecB = coopMatShape / (sizeof(uint4) / sizeof(half));
    const int tileStrideVec = coopMatShape / (sizeof(uint4) / sizeof(DType));
    const int mPadded = (int(outputSize) + 1) & ~1;
    const int bytesForSumReduceRows = !isTraining ? 0 :
        (subgroupCount * SubgroupSize * heightInVecB + subgroupCount * tileStrideVec * coopMatShape) * sizeof(uint4)
        + ((subgroupCount * mPadded * sizeof(DType) + sizeof(uint4) - 1) / sizeof(uint4)) * sizeof(uint4);

    result[index++] = max(max(max(bytesForMMA, bytesForOuterProductStore), bytesForCrossWarpReduction), bytesForSumReduceRows);
}


uint verifySize<ExecutionMode Mode, each T, each U>(expand each T a, expand each U b)
    where T == uint
    where U == uint
{
    uint index = 0;
    uint resBuffer[countof(T)];
    expand computeOneLayer<Mode>(each a, each b, index, resBuffer);

    int maxSize = 0;

    [ForceUnroll]
    for (int i = 0; i < countof(T); i++)
    {
        maxSize = max(maxSize, resBuffer[i]);
    }

    return maxSize;
}

// randomly generated layer configurations

// hidden layer count = 0
#define TEST_CASE_1 5U, 9U
#define TEST_CASE_1_PAIR_1 5U
#define TEST_CASE_1_PAIR_2 9U

// hidden layer count = 1
#define TEST_CASE_2 17U, 23U, 3U
#define TEST_CASE_2_PAIR_1 17U, 23U
#define TEST_CASE_2_PAIR_2 23U, 3U

// hidden layer count = 2
#define TEST_CASE_3 1U, 2U, 3U, 4U
#define TEST_CASE_3_PAIR_1 1U, 2U, 3U
#define TEST_CASE_3_PAIR_2 2U, 3U, 4U

// hidden layer count = 3
#define TEST_CASE_4 7U, 18U, 29U, 42U, 57U
#define TEST_CASE_4_PAIR_1 7U, 18U, 29U, 42U
#define TEST_CASE_4_PAIR_2 18U, 29U, 42U, 57U

// hidden layer count = 4
#define TEST_CASE_5 3U, 11U, 24U, 37U, 49U, 62U
#define TEST_CASE_5_PAIR_1 3U, 11U, 24U, 37U, 49U
#define TEST_CASE_5_PAIR_2 11U, 24U, 37U, 49U, 62U

// hidden layer count = 5
#define TEST_CASE_6 38U, 7U, 59U, 14U, 46U, 3U, 27U
#define TEST_CASE_6_PAIR_1 38U, 7U, 59U, 14U, 46U, 3U
#define TEST_CASE_6_PAIR_2 7U, 59U, 14U, 46U, 3U, 27U

// hidden layer count = 6
#define TEST_CASE_7 41U, 6U, 58U, 13U, 29U, 2U, 47U, 35U
#define TEST_CASE_7_PAIR_1 41U, 6U, 58U, 13U, 29U, 2U, 47U
#define TEST_CASE_7_PAIR_2 6U, 58U, 13U, 29U, 2U, 47U, 35U

// hidden layer count = 7
#define TEST_CASE_8 3U, 21U, 45U, 12U, 56U, 7U, 39U, 50U, 8U
#define TEST_CASE_8_PAIR_1 3U, 21U, 45U, 12U, 56U, 7U, 39U, 50U
#define TEST_CASE_8_PAIR_2 21U, 45U, 12U, 56U, 7U, 39U, 50U, 8U

// hidden layer count = 8
#define TEST_CASE_9 11U, 27U, 4U, 33U, 58U, 19U, 42U, 6U, 31U, 22U
#define TEST_CASE_9_PAIR_1 11U, 27U, 4U, 33U, 58U, 19U, 42U, 6U, 31U
#define TEST_CASE_9_PAIR_2 27U, 4U, 33U, 58U, 19U, 42U, 6U, 31U, 22U

// hidden layer count = 9
#define TEST_CASE_10 5U, 48U, 2U, 37U, 14U, 51U, 9U, 28U, 41U, 17U, 60U
#define TEST_CASE_10_PAIR_1 5U, 48U, 2U, 37U, 14U, 51U, 9U, 28U, 41U, 17U
#define TEST_CASE_10_PAIR_2 48U, 2U, 37U, 14U, 51U, 9U, 28U, 41U, 17U, 60U

// hidden layer count = 10
#define TEST_CASE_11 8U, 36U, 50U, 3U, 22U, 57U, 14U, 44U, 6U, 31U, 12U, 59U
#define TEST_CASE_11_PAIR_1 8U, 36U, 50U, 3U, 22U, 57U, 14U, 44U, 6U, 31U, 12U
#define TEST_CASE_11_PAIR_2 36U, 50U, 3U, 22U, 57U, 14U, 44U, 6U, 31U, 12U, 59U

// hidden layer count = 11
#define TEST_CASE_12 7U, 49U, 18U, 5U, 33U, 41U, 2U, 28U, 11U, 54U, 19U, 47U, 23U
#define TEST_CASE_12_PAIR_1 7U, 49U, 18U, 5U, 33U, 41U, 2U, 28U, 11U, 54U, 19U, 47U
#define TEST_CASE_12_PAIR_2 49U, 18U, 5U, 33U, 41U, 2U, 28U, 11U, 54U, 19U, 47U, 23U

// hidden layer count = 12
#define TEST_CASE_13 12U, 3U, 56U, 7U, 21U, 49U, 15U, 28U, 5U, 37U, 11U, 44U, 6U, 30U
#define TEST_CASE_13_PAIR_1 12U, 3U, 56U, 7U, 21U, 49U, 15U, 28U, 5U, 37U, 11U, 44U, 6U
#define TEST_CASE_13_PAIR_2 3U, 56U, 7U, 21U, 49U, 15U, 28U, 5U, 37U, 11U, 44U, 6U, 30U

// hidden layer count = 13
#define TEST_CASE_14 9U, 27U, 41U, 6U, 32U, 18U, 50U, 4U, 39U, 12U, 23U, 55U, 7U, 34U, 16U
#define TEST_CASE_14_PAIR_1 9U, 27U, 41U, 6U, 32U, 18U, 50U, 4U, 39U, 12U, 23U, 55U, 7U, 34U
#define TEST_CASE_14_PAIR_2 27U, 41U, 6U, 32U, 18U, 50U, 4U, 39U, 12U, 23U, 55U, 7U, 34U, 16U

// hidden layer count = 14
#define TEST_CASE_15 2U, 45U, 10U, 51U, 8U, 36U, 19U, 5U, 28U, 43U, 12U, 56U, 7U, 34U, 21U, 48U
#define TEST_CASE_15_PAIR_1 2U, 45U, 10U, 51U, 8U, 36U, 19U, 5U, 28U, 43U, 12U, 56U, 7U, 34U, 21U
#define TEST_CASE_15_PAIR_2 45U, 10U, 51U, 8U, 36U, 19U, 5U, 28U, 43U, 12U, 56U, 7U, 34U, 21U, 48U

// hidden layer count = 15
#define TEST_CASE_16 14U, 3U, 49U, 8U, 37U, 6U, 28U, 11U, 41U, 2U, 33U, 17U, 55U, 9U, 44U, 12U, 29U
#define TEST_CASE_16_PAIR_1 14U, 3U, 49U, 8U, 37U, 6U, 28U, 11U, 41U, 2U, 33U, 17U, 55U, 9U, 44U, 12U
#define TEST_CASE_16_PAIR_2 3U, 49U, 8U, 37U, 6U, 28U, 11U, 41U, 2U, 33U, 17U, 55U, 9U, 44U, 12U, 29U

#define RunTest(N) \
do { \
    uint expectedValue = verifySize<ExecutionMode.Inference>(TEST_CASE_##N##_PAIR_1, TEST_CASE_##N##_PAIR_2); \
    uint actualValue = SharedMemorySizeInference.OfLayer##N<TEST_CASE_##N>.Bytes; \
    bool isCorrect = expectedValue == actualValue; \
    actualValue = SharedMemorySizeTraining.OfLayer##N<TEST_CASE_##N>.Bytes; \
    expectedValue = verifySize<ExecutionMode.Training>(TEST_CASE_##N##_PAIR_1, TEST_CASE_##N##_PAIR_2); \
    isCorrect = isCorrect && (expectedValue == actualValue); \
    outputBuffer[N-1] = isCorrect ? 1U : 0U; \
} while (false)


[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid: SV_DispatchThreadID)
{
    __target_switch
    {
    case cuda:
        typealias SharedMemorySizeInference = SharedMemorySize<DType, TargetEnum.CUDA, ExecutionMode.Inference, SubgroupSize, BatchSize/SubgroupSize>;
        typealias SharedMemorySizeTraining = SharedMemorySize< DType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize/SubgroupSize>;
        RunTest(1);
        RunTest(2);
        RunTest(3);
        RunTest(4);
        RunTest(5);
        RunTest(6);
        RunTest(7);
        RunTest(8);
        RunTest(9);
        RunTest(10);
        RunTest(11);
        RunTest(12);
        RunTest(13);
        RunTest(14);
        RunTest(15);
        RunTest(16);
    case spirv:
        typealias SharedMemorySizeInference = SharedMemorySize<DType, TargetEnum.SPIR_V, ExecutionMode.Inference, SubgroupSize, BatchSize/SubgroupSize>;
        typealias SharedMemorySizeTraining = SharedMemorySize< DType, TargetEnum.SPIR_V, ExecutionMode.Training, SubgroupSize, BatchSize/SubgroupSize>;
        RunTest(1);
        RunTest(2);
        RunTest(3);
        RunTest(4);
        RunTest(5);
        RunTest(6);
        RunTest(7);
        RunTest(8);
        RunTest(9);
        RunTest(10);
        RunTest(11);
        RunTest(12);
        RunTest(13);
        RunTest(14);
        RunTest(15);
        RunTest(16);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
