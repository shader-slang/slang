// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)

import neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set weightBuffer = ubuffer(data=[0], stride=4, count=4096)
uniform DType* weightBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(data=[0], stride=4, count=64)
uniform DType* biasBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

groupshared uint4 s_sharedMemoryA[256];
groupshared uint4 s_sharedMemoryB[256];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

// Clear the weight buffer region used by a PaddedM x PaddedK tiled matrix.
void clearWeightBuffer<int PaddedM, int PaddedK>(uint tid)
{
    static const int TotalElements = PaddedM * PaddedK;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            weightBuffer[index] = DType(0);
    }
}

// Compute the tiled layout offset for element (row, col) in a PaddedM x PaddedK matrix.
int tiledOffset<int PaddedK>(int row, int col)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedK / TileSize;
    int tileRow = row / TileSize;
    int tileCol = col / TileSize;
    int localRow = row % TileSize;
    int localCol = col % TileSize;
    int tileIndex = tileRow * NTileCols + tileCol;
    return tileIndex * TileSize * TileSize + localRow * TileSize + localCol;
}

// Fill an OutputSize x InputSize weight matrix with 0.5 in tiled layout.
// The matrix is padded to PaddedM x PaddedK (multiples of 16); padding is zero.
void fillWeightMatrixTiled<int InputSize, int OutputSize, int PaddedK, int PaddedM>(uint tid)
{
    // Clear the entire padded region first.
    clearWeightBuffer<PaddedM, PaddedK>(tid);
    AllMemoryBarrierWithGroupSync();

    // Fill the valid region with 0.5. Each thread iterates over rows.
    static const int NumRowIters = (OutputSize + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumRowIters; iter++)
    {
        int row = iter * BatchSize + tid;
        if (row >= OutputSize)
            break;

        for (int col = 0; col < InputSize; col++)
        {
            int offset = tiledOffset<PaddedK>(row, col);
            weightBuffer[offset] = DType(0.5);
        }
    }
}

DType[TiledMMAHelper<DType, PaddedInputSize, PaddedOutputSize, SubgroupSize, Target>.TileInfoT.Uint4AlignedM]
    testMatVecMul<int InputSize, int OutputSize, int PaddedInputSize, int PaddedOutputSize, TargetEnum Target>(uint tid)
{
    typealias Address = PointerAddress<DType>;
    Address address = Address(weightBuffer);

    typealias MMA = TiledMMAHelper<DType, PaddedInputSize, PaddedOutputSize, SubgroupSize, Target, false>;
    const int InSize = MMA.TileInfoT.Uint4AlignedK;
    const int OutSize = MMA.TileInfoT.Uint4AlignedM;

    DType inputVector[InSize] = {};
    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = DType(tid + 1);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

#if !TEST_BIAS
    let res = MMA.mmaTiledLayout<DType, Address, DType[InSize], DType[OutSize], DType, false>(
        inputVector, ptrA, ptrB, ptrC, address, none);
#else
    Address biasAddress = Address(biasBuffer);
    let res = MMA.mmaTiledLayout<DType, Address, DType[InSize], DType[OutSize], DType, true>(
        inputVector, ptrA, ptrB, ptrC, address, biasAddress);
#endif

    return res;
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    DType[TiledMMAHelper<DType, PaddedInputSize, PaddedOutputSize, SubgroupSize, TargetEnum.CUDA>.TileInfoT.Uint4AlignedM] outputVector;
    __target_switch
    {
    case cuda:
        outputVector = testMatVecMul<InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TargetEnum.CUDA>(tid);
        break;
    case spirv:
        outputVector = testMatVecMul<InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TargetEnum.SPIR_V>(tid);
        break;
    }

    bool res = true;
    if (tid < OutputSize)
    {
        for (int i = 0; i < OutputSize; i++)
        {
#if !TEST_BIAS
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5);
#else
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5) + DType(i);
#endif
            if (!equals(outputVector[i], expected))
            {
                if (tid == 0)
                    printf("FAIL resIndex=%d i=%d expected=%f got=%f\n", resIndex, i, float(expected), float(outputVector[i]));
                res = false;
                break;
            }
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    // 3x5 (InputSize=3, OutputSize=5, padded to 16x16)
    {
        fillWeightMatrixTiled<3, 5, 16, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
    }

    // 11x18 (padded to 16x32)
    {
        fillWeightMatrixTiled<11, 18, 16, 32>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
    }

    // 23x9 (padded to 32x16)
    {
        fillWeightMatrixTiled<23, 9, 32, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
    }

    // 35x3 (padded to 48x16)
    {
        fillWeightMatrixTiled<35, 3, 48, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
    }

    // 17x64 (padded to 32x64)
    {
        fillWeightMatrixTiled<17, 64, 32, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
