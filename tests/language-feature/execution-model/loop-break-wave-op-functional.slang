// Corner case: break inside a loop causes progressive thread attrition.
// All threads start in the loop body. At each iteration, one more thread
// breaks out. WaveActiveCountBits(true) BEFORE the break counts active threads.
//
// Iteration 0: all 4 active, thread 0 breaks → count=4 for thread 0
// Iteration 1: 3 active, thread 1 breaks → count=3 for thread 1
// Iteration 2: 2 active, thread 2 breaks → count=2 for thread 2
// Iteration 3: 1 active, thread 3 breaks → count=1 for thread 3

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int result = 0;

    for (int i = 0; i < 4; ++i)
    {
        // All still-looping threads execute this wave op together
        int activeCount = int(WaveActiveCountBits(true));

        // Thread whose index matches this iteration breaks out
        if (i == idx)
        {
            result = activeCount;
            break;
        }
    }

    outputBuffer[idx] = result;
}

// Thread 0: breaks at i=0, all 4 active → 4
// Thread 1: breaks at i=1, threads {1,2,3} active → 3
// Thread 2: breaks at i=2, threads {2,3} active → 2
// Thread 3: breaks at i=3, thread {3} active → 1
// CHECK: 4
// CHECK: 3
// CHECK: 2
// CHECK: 1
