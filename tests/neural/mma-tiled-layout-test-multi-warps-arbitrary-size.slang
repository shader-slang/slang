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

import slang.neural;
import common;
#pragma warning(disable: 41017)

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

// TEST_INPUT: set buffers.weightBuffer = ubuffer(stride=4, count=4096)
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

void fillWeightMatrixTiled<int InputSize, int OutputSize>(uint tid)
{
    fillForwardWeightTiled<DType, InputSize, OutputSize, BatchSize>(tid, buffers.weightBuffer, DType(0.125f));
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

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
        DType scaleFactor = DType(0.125f);
#if !TEST_BIAS
        DType expected = inputValue * DType(InputSize) * scaleFactor;
#else
        DType expected = inputValue * DType(InputSize) * scaleFactor + DType(i);
#endif
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }

    bool finalResult = collectResults<BatchSize>(tid, res);
    if (tid == 0)
    {
        outputBuffer[resIndex] = finalResult ? 1 : 0;
    }
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    initBias<DType, 64, BatchSize>(tid, buffers.biasBuffer);

    // 3x5 (padded to 16x16)
    {
        fillWeightMatrixTiled<3, 5>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    // 11x18 (padded to 16x32)
    {
        fillWeightMatrixTiled<11, 18>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    // 23x9 (padded to 32x16)
    {
        fillWeightMatrixTiled<23, 9>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    // 35x3 (padded to 48x16)
    {
        fillWeightMatrixTiled<35, 3>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }

    // 17x64 (padded to 32x64)
    {
        fillWeightMatrixTiled<17, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
}
