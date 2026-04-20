//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;
groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    // Round 1: write idx*10, read neighbor
    sharedData[idx] = int(idx) * 10;
    GroupMemoryBarrierWithGroupSync();
    int round1 = sharedData[(int(idx) + 1) % 4];

    // Round 2: write round1*2, read neighbor
    GroupMemoryBarrierWithGroupSync();
    sharedData[idx] = round1 * 2;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[idx] = sharedData[(int(idx) + 1) % 4];
}

// Round 1 reads: thread[0]=10, thread[1]=20, thread[2]=30, thread[3]=0
// Round 2 writes: thread[0]=20, thread[1]=40, thread[2]=60, thread[3]=0
// Round 2 reads neighbor: thread[0]=40, thread[1]=60, thread[2]=0, thread[3]=20

// CHECK: 40
// CHECK: 60
// CHECK: 0
// CHECK: 20
