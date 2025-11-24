// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=2, count=1024):out, name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

static const int InputSize = 32;
static const int OutputSize = 32;
static const int SubgroupSize = 32;

groupshared uint4 s_sharedMemoryA[OutputSize * 2];
typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA(uint tid)
{
    // Only test the within a warp
    if (tid >= 32)
        return;

    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, InputSize, OutputSize, SubgroupSize>.LoadShA<Storage>(sharedMemoryA, 0, storage, 0);
    AllMemoryBarrierWithWaveSync();
}

void writeBackToOutput(uint tid)
{
    if (tid >= OutputSize * 2)
        return;

    // One thread writes 8 half values to output buffer.
    uint4 values = s_sharedMemoryA[tid];
    uint index = tid * 8;

    uint4 element = values;
    for (int i = 0; i < 4; i++)
    {
        uint value = element[i];
        // printf("value: %x\n", element[i]);

        uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
        uint16_t b = (uint16_t)(value & 0xFFFF);

        half aa = bit_cast<half>(a);
        half bb = bit_cast<half>(b);
        outputBuffer[index + 2 * i] = aa;
        outputBuffer[index + 2 * i + 1] = bb;
    }
}


[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);

    testLoadShA(tid);

    writeBackToOutput(tid);

    // int index = tid.y * 32 + tid.x;
    // outputBuffer[index] = inputBuffer[index];
}
