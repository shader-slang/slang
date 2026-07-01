// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

struct TestBuffers
{
    RWStructuredBuffer<DType>.Handle weightBuffer;
    RWStructuredBuffer<DType>.Handle biasBuffer;
}

// TEST_INPUT: set buffers.weightBuffer = ubuffer(data=[0], stride=4, count=4096)
// TEST_INPUT: set buffers.biasBuffer = ubuffer(data=[0], stride=4, count=64)
ParameterBlock<TestBuffers> buffers;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

void testImpl<int InputSize, int OutputSize, TargetEnum Target>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTargetTile<InputSize, Target>.Value;
    static const int PaddedOutputSize = PadToTargetTile<OutputSize, Target>.Value;

    typealias InputVec = WaveTangledVector<DType, TestShMemSize, PaddedInputSize, SubgroupSize, 2>;
    InputVec inputData = InputVec(DType(0));
    DType inputValue = DType(WaveGetLaneIndex() + 1 + tid / 32);
    for (int i = 0; i < InputSize; i++)
        inputData[i] = inputValue;

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 2, false>(
        inputData, buffers.weightBuffer, Optional<RWStructuredBuffer<DType>.Handle>(buffers.biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 2, false>(
        inputData, buffers.weightBuffer, none);
#endif

    bool res = true;
    for (int i = 0; i < OutputSize; i++)
    {
#if !TEST_BIAS
        DType expected = inputValue;
#else
        DType expected = inputValue + DType(i);
#endif
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }

    bool finalResult = collectResults<BatchSize>(tid, res);
    if (tid == 0)
        outputBuffer[resIndex] = finalResult ? 1 : 0;
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    __target_switch
    {
    case cuda:
        testImpl<InputSize, OutputSize, TargetEnum.CUDA>(tid, resIndex);
        return;
    case spirv:
        testImpl<InputSize, OutputSize, TargetEnum.SPIR_V>(tid, resIndex);
        return;
    case metal:
        testImpl<InputSize, OutputSize, TargetEnum.Metal>(tid, resIndex);
        return;
    }
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    initBias<DType, 64, BatchSize>(tid, buffers.biasBuffer);

    // 4x4 (target-padded)
    {
        identityWeightMatrixTiled<DType, 4, BatchSize, false>(tid, buffers.weightBuffer);
        AllMemoryBarrierWithGroupSync();
        test<4, 4>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    // 8x8 (target-padded)
    {
        identityWeightMatrixTiled<DType, 8, BatchSize, false>(tid, buffers.weightBuffer);
        AllMemoryBarrierWithGroupSync();
        test<8, 8>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    // 16x16 (target-padded)
    {
        identityWeightMatrixTiled<DType, 16, BatchSize, false>(tid, buffers.weightBuffer);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    // 32x32 (target-padded)
    {
        identityWeightMatrixTiled<DType, 32, BatchSize, false>(tid, buffers.weightBuffer);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }

    // 64x64 (target-padded)
    {
        identityWeightMatrixTiled<DType, 64, BatchSize, false>(tid, buffers.weightBuffer);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 4);
    }

    // BUFFER: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
}
