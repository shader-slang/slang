// Tiled-layout MMA transpose test with arbitrary sizes (single warp).
// Sizes are padded to multiples of 16 for TiledMMAHelper.

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1

// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set portableWeightBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle portableWeightBuffer;

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

void fillWeightMatrixTiled<int InputSize, int OutputSize, int PaddedInputSize, int PaddedOutputSize, bool Clear>(uint tid)
{
    static const int PhysicalCount = PaddedInputSize * PaddedOutputSize + PaddedOutputSize;

    if (Clear)
    {
        static const int NumIters = (PhysicalCount + BatchSize - 1) / BatchSize;
        for (int iter = 0; iter < NumIters; iter++)
        {
            int index = iter * BatchSize + tid;
            if (index < PhysicalCount)
                inputBuffer[index] = DType(0);
        }
        return;
    }

    static const int LogicalCount = InputSize * OutputSize;
    static const int NumLogicalIters = (LogicalCount + BatchSize - 1) / BatchSize;
    for (int iter = 0; iter < NumLogicalIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < LogicalCount)
        {
            int row = index / InputSize;
            int col = index - row * InputSize;
            int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
            portableWeightBuffer[index] = DType(scaleFactor * (col + 1));
        }
    }

    AllMemoryBarrierWithGroupSync();

    let portable = BindlessAddress<DType>(portableWeightBuffer);
    let optimal = BindlessAddress<DType>(inputBuffer);
    __target_switch
    {
    case cuda:
        NetworkParameterLayoutConverter<DType, TargetEnum.CUDA, 0, InputSize, OutputSize>.toOptimalLayout(portable, optimal);
    case spirv:
        NetworkParameterLayoutConverter<DType, TargetEnum.SPIR_V, 0, InputSize, OutputSize>.toOptimalLayout(portable, optimal);
    case metal:
        NetworkParameterLayoutConverter<DType, TargetEnum.Metal, 0, InputSize, OutputSize>.toOptimalLayout(portable, optimal);
    }
    AllMemoryBarrierWithGroupSync();
}
void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    typealias InputVec = WaveTangledVector<DType, TestShMemSize, PaddedOutputSize, SubgroupSize, 1>;
    InputVec inputData = InputVec(DType(0));
    for (int i = 0; i < OutputSize; i++)
        inputData[i] = DType(tid + 1);

    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedOutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, true>(
        inputData, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        DType expected = DType(scaleFactor * (i + 1.0f) * (tid + 1.0f));
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    {
        fillWeightMatrixTiled<3, 5, 16, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
        fillWeightMatrixTiled<3, 5, 16, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<11, 18, 16, 32, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
        fillWeightMatrixTiled<11, 18, 16, 32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<23, 9, 32, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
        fillWeightMatrixTiled<23, 9, 32, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<35, 3, 48, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
        fillWeightMatrixTiled<35, 3, 48, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<17, 64, 32, 64, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
        fillWeightMatrixTiled<17, 64, 32, 64, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
}
