// Corner case: early return removes threads, then remaining threads do wave ops.
// Thread 0 returns early. Threads {1,2,3} continue and do WaveActiveMin.
// WaveActiveMin({1,2,3}) should be 1 (thread 0 is inactive).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);

    // Thread 0 returns early with a marker value
    if (idx == 0)
    {
        outputBuffer[idx] = -1;
        return;
    }

    // Threads {1,2,3} are active: WaveActiveMin should be 1
    int minVal = WaveActiveMin(idx);
    outputBuffer[idx] = minVal;
}

// Thread 0: returned early → -1
// Threads 1,2,3: WaveActiveMin({1,2,3}) = 1
// CHECK: -1
// CHECK: 1
// CHECK: 1
// CHECK: 1
