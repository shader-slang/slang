//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int acc = 0;
    int limit = 5 + (idx & 1);
    for (int i = 0; i < limit; ++i)
    {
        acc += i;
    }
    // Thread 0: loops 5 times, acc=0+1+2+3+4=10
    // Thread 1: loops 6 times, acc=0+1+2+3+4+5=15
    // Thread 2: loops 5 times, acc=10
    // Thread 3: loops 6 times, acc=15
    // Sum = 10+15+10+15 = 50
    outputBuffer[idx] = WaveActiveSum(acc);
}

// CHECK: 50
// CHECK: 50
// CHECK: 50
// CHECK: 50
