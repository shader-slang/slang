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
// Since tileA is 32×32 bytes, each thread loads 16 elements per row.
// test1: Expected output for the first tile:
//
//   0    1    2    3   ...   15
//  32   33   34   35   ...   47
//  ...
// 960  961  962  963  ...  975
// 992  993  994  995  ... 1007
//
// test2: Expected output for the second tile:
//
//  16   17   18   19   ...   31
//  48   49   50   51   ...   63
//  ...
// 1008 1009 1010 1011  ... 1023

// ============================================================================
// MATRIX B - Loaded from Thread-Local Vector
// ============================================================================
// Each thread constructs a local vector where:
//   element[i] = tid * 32 + i, for i = 0, 1, 2, ... 31
//
// Since tileB is 32×32 bytes, each thread loads 16 elements per column.
// test1: Expected output for the first tile (column-major):
//
//   0   32   ...  960  992
//   1   33   ...  961  993
//   2   34   ...  962  994
//   3   35   ...  963  995
//  ...
//  15   47   ...  975 1007

// test2: Expected output for the second tile (column-major):
//
//  16   48   ...  1008 1040
//  17   49   ...  1009 1041
//  18   50   ...  1010 1042
//  19   51   ...  1011 1043
//  ...
//  31   63   ...  1023 1055

// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=4):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;


void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

static const int InputSize = 32;
static const int OutputSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (OutputSize * 2 + SubgroupSize * 2) * sizeof(uint4);
}
typealias TestPool = SharedMemoryPool<TestShMemSize>;
static const uint OffsetA = 0;
static const uint OffsetB = OutputSize * 2;

void testLoadShA<TargetEnum Target>(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target, TestPool, false>.loadShA<half, Storage>(OffsetA, tileIndex, storage, 0);
    GroupMemoryBarrierWithWaveSync();
}

void testLoadShB<TargetEnum Target>(uint tid, uint tileIndex)
{
    // Construct the input vector as follow:
    // x = tid * 32 + 0, 1, 2, 3 ... 31
    half inputVector[InputSize];
    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = half(tid * InputSize + i);
    }

    // This test only has one subgroup, so the subgroup index is always 0.
    MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target, TestPool, false>.loadVectorToShB(OffsetB, tileIndex, 0, inputVector);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput(uint tid, uint size, uint sharedMemOffset, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * 32 + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread
    for (int i = 0; i < 2; i++)
    {
        uint4 values = TestPool.data[sharedMemOffset + tid * 2 + i];
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

void test1<TargetEnum Target>(uint tid)
{
    if (tid >= SubgroupSize)
        return;

    testLoadShA<Target>(tid, 0);

    // serialRead(tid, OffsetA);

    bool res = verifiedOutput(tid, OutputSize, OffsetA, 0);
    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[0] = res ? 1 : 0;
    // BUFFER: 1

    testLoadShB<Target>(tid, 0);

    bool res1 = verifiedOutput(tid, InputSize, OffsetB, 0);
    res1 = WaveActiveAllTrue(res1);
    if (tid == 0)
        outputBuffer[1] = res1 ? 1 : 0;
    // BUFFER-NEXT: 1
}

void test2<TargetEnum Target>(uint tid)
{
    if (tid >= SubgroupSize)
        return;

    testLoadShA<Target>(tid, 1);

    bool res = verifiedOutput(tid, OutputSize, OffsetA, 1);
    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[2] = res ? 1 : 0;
    // BUFFER-NEXT: 1

    testLoadShB<Target>(tid, 1);

    bool res1 = verifiedOutput(tid, InputSize, OffsetB, 1);
    res1 = WaveActiveAllTrue(res1);
    if (tid == 0)
        outputBuffer[3] = res1 ? 1 : 0;
    // BUFFER-NEXT: 1
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, uint sharedMemOffset)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    for (int id = 0; id < 32; id++)
    {
        printf("tid: %d\n", id);
        for (int i = 0; i < 2; i++)
        {
            uint4 values = TestPool.data[sharedMemOffset + id * 2 + i];
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

    __target_switch
    {
    case cuda:
        test1<TargetEnum.CUDA>(tid);
        test2<TargetEnum.CUDA>(tid);
        break;
    case spirv:
        test1<TargetEnum.SPIR_V>(tid);
        test2<TargetEnum.SPIR_V>(tid);
        break;
    }
}
