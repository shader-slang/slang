// Tiled-layout MMA transpose test (multi warps, BatchSize=64 = 2 warps).
// Tests TiledMMAHelper with TransposeA=true: computes W^T * x where W is in tiled layout.

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

void fillWeightMatrixTiled<int InputSize, int OutputSize, int PaddedInputSize, int PaddedOutputSize, bool Clear>(uint tid)
{
    fillTransposeWeightTiled<DType, OutputSize, InputSize, BatchSize, Clear>(tid, inputBuffer);
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    DType inputData[OutputSize];
    for (int i = 0; i < OutputSize; i++)
        inputData[i] = DType(tid + 1);

    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 2, true>(
        inputData, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        int intVal = (i + 1) * (tid + 1);
        DType expected = DType(scaleFactor) * DType(intVal);
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

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        fillWeightMatrixTiled<16, 16, 16, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 0);
        fillWeightMatrixTiled<16, 16, 16, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<32, 32, 32, 32, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 1);
        fillWeightMatrixTiled<32, 32, 32, 32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<64, 64, 64, 64, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 2);
        fillWeightMatrixTiled<64, 64, 64, 64, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
