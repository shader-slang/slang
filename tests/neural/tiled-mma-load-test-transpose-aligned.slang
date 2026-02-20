// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly.
//
// ============================================================================
// MATRIX A - Loaded from Global Memory
// ============================================================================
// We construct a 32x32 matrix in row-major order:
//
//   Column:  0    1    2    3   ...  15   16   17   ...   31
//   Row 0:   0    1    2    3   ...  15   16   17   ...   31
//   Row 1:  32   33   34   35   ...  47   48   49   ...   63
//   ...
//   Row 30: 960  961  962  963  ...  975  976  977  ...  991
//   Row 31: 992  993  994  995  ... 1007 1008 1009  ... 1023
//
// In the transpose mode, each tile still load a column of CoopMat of A^T, but the
// memory layout is in column major. Therefore, each load will results in a
// AlignedK x WMMA_TileWidth (16 for half type) matrix.

// test1: Expected output for the first tile:
// col0:   0    1    2    3   ...   31
// col1:  32   33   34   35   ...   63
// ...
// col15: 480  481  482  483  ...  511

// test2: Expected output for the second tile:
// col0:  512  513  514  515  ...  543
// col1:  544  545  546  547  ...  575
// ...
// col15: 992  993  994  995  ... 1023


// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT: set inputBuffer = ubuffer(stride=2, count=1024)
uniform RWStructuredBuffer<half>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=2):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;


void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

static const int InputSize = 32;
static const int OutputSize = 32;
static const int SubgroupSize = 32;

// Tile A is size of OutputSize * 32 bytes
groupshared uint4 s_sharedMemoryA[OutputSize * 2];

// Tile B is size of 32 x SubgroupSize bytes
groupshared uint4 s_sharedMemoryB[SubgroupSize * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA<TargetEnum Target>(uint tid, uint tileIndex)
{
    typealias Address = BindlessAddress<half>;

    Address address = Address(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target, true>.loadShA<half, Address>(sharedMemoryA, tileIndex, address);
    GroupMemoryBarrierWithWaveSync();
}


bool verifiedOutput(uint tid, uint size, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one column of the shared memory.
    // The tile width of A is just 16 for half type.
    const int WMMA_TileWidth = 16;
    const int WMMA_TileHeight = 16;
    if (tid >= WMMA_TileWidth)
        return true;

    const int AlignedK = ((InputSize + WMMA_TileWidth - 1) / WMMA_TileWidth) * WMMA_TileWidth;
    const int TileSize = AlignedK * WMMA_TileWidth;
    const int ElementCountPerVector = sizeof(uint4) / sizeof(half);
    const int NumVectorsPerColumn = AlignedK / ElementCountPerVector;

    half expected = half(tid * AlignedK + tileIndex * TileSize);
    bool res = true;
    uint indexInTile = tid * NumVectorsPerColumn;  // AlignedK half per thread

    for (int i = 0; i < NumVectorsPerColumn; i++)
    {
        uint4 values = sharedMem[indexInTile + i];
        uint4 element = values;
        for (int j = 0; j < 4; j++)
        {
            uint value = element[j];

            uint16_t a = (uint16_t)(value & 0xFFFF);
            uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

            half aa = bit_cast<half>(a);
            half bb = bit_cast<half>(b);

            if (aa != expected)
            {
                res = false;
                break;
            }
            expected += half(1.0f);
            if (bb != expected)
            {
                res = false;
                break;
            }
            expected += half(1.0f);
        }
    }

    return res;
}

void test(uint tid, uint tileIndex, uint resIndex)
{
    if (tid >= SubgroupSize)
        return;

    __target_switch
    {
    case cuda:
        testLoadShA<TargetEnum.CUDA>(tid, tileIndex);
        break;
    case spirv:
        testLoadShA<TargetEnum.SPIR_V>(tid, tileIndex);
        break;
    }
    // serialRead(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput(tid, OutputSize, __getAddress(s_sharedMemoryA[0]), tileIndex);
    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    // In transpose mode, tile is in column major, and each length is AlignedK length
    for (int id = 0; id < 16; id++)
    {
        printf("col: %d\n", id);
        for (int i = 0; i < 4; i++)
        {
            uint4 values = sharedMem[id * 4 + i];
            uint4 element = values;
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);

    test(tid, 0, 0);
    // BUFFER: 1
    test(tid, 1, 1);
    // BUFFER-NEXT: 1
}
