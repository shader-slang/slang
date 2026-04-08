// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import slang.neural;
#pragma warning(disable: 41017)

// TEST_INPUT: set inputBuffer = ubuffer(stride=2, count=1024)
uniform RWStructuredBuffer<half>.Handle inputBuffer;

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
static const uint OffsetA = 0;
static const uint OffsetB = OutputSize * 2;

bool verifiedOutput(uint tid, uint size, uint sharedMemOffset, uint tileIndex)
{
    half expected = half(tid * 32 + tileIndex * 16);
    bool res = true;
    for (int i = 0; i < 2; i++)
    {
        uint4 values = neural_unittest.readSharedMemory<TestShMemSize>(sharedMemOffset + tid * 2 + i);
        uint4 element = values;
        for (int j = 0; j < 4; j++)
        {
            uint value = element[j];
            uint16_t a = (uint16_t)(value & 0xFFFF);
            uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);
            half aa = bit_cast<half>(a);
            half bb = bit_cast<half>(b);
            if (aa != expected) { res = false; break; }
            expected += half(1.0f);
            if (bb != expected) { res = false; break; }
            expected += half(1.0f);
        }
    }
    return res;
}

void test1(uint tid)
{
    if (tid >= SubgroupSize)
        return;

    neural_unittest.testLoadShA<InputSize, OutputSize, TestShMemSize, false, 1>(OffsetA, 0, inputBuffer);

    bool res = verifiedOutput(tid, OutputSize, OffsetA, 0);
    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[0] = res ? 1 : 0;
    // BUFFER: 1

    half inputVector[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputVector[i] = half(tid * InputSize + i);
    neural_unittest.testLoadShB<InputSize, OutputSize, TestShMemSize, 1>(OffsetB, 0, 0, inputVector);

    bool res1 = verifiedOutput(tid, InputSize, OffsetB, 0);
    res1 = WaveActiveAllTrue(res1);
    if (tid == 0)
        outputBuffer[1] = res1 ? 1 : 0;
    // BUFFER-NEXT: 1
}

void test2(uint tid)
{
    if (tid >= SubgroupSize)
        return;

    neural_unittest.testLoadShA<InputSize, OutputSize, TestShMemSize, false, 1>(OffsetA, 1, inputBuffer);

    bool res = verifiedOutput(tid, OutputSize, OffsetA, 1);
    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[2] = res ? 1 : 0;
    // BUFFER-NEXT: 1

    half inputVector[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputVector[i] = half(tid * InputSize + i);
    neural_unittest.testLoadShB<InputSize, OutputSize, TestShMemSize, 1>(OffsetB, 1, 0, inputVector);

    bool res1 = verifiedOutput(tid, InputSize, OffsetB, 1);
    res1 = WaveActiveAllTrue(res1);
    if (tid == 0)
        outputBuffer[3] = res1 ? 1 : 0;
    // BUFFER-NEXT: 1
}

[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);
    test1(tid);
    test2(tid);
}
