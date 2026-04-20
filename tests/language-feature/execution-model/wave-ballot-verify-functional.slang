//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    bool isEven = (idx & 1) == 0;
    uint4 ballot = WaveActiveBallot(isEven);
    int count = int(WaveActiveCountBits(isEven));
    outputBuffer[idx * 2] = int(ballot.x);
    outputBuffer[idx * 2 + 1] = count;
}

// ballot.x = 0x5 = 5 for all threads, count = 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
// CHECK: 5
// CHECK: 2
