// Functional test for loop divergence and reconvergence.
// From the docs: "Divergence occurs when some threads exit a loop while the
// rest continue. Reconvergence occurs when all threads have exited the loop."
//
// Each thread loops a different number of times based on idx & 1:
//   Thread 0: 5 iters, acc = 0+1+2+3+4 = 10
//   Thread 1: 6 iters, acc = 0+1+2+3+4+5 = 15
//   Thread 2: 5 iters, acc = 10
//   Thread 3: 6 iters, acc = 15
//
// After loop reconvergence, WaveActiveSum should see all 4 threads:
//   sum = 10+15+10+15 = 50

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);

    // Threads iterate 5 or 6 times depending on parity
    int numLoops = 5 + (idx & 1);
    int acc = 0;
    for (int i = 0; i < numLoops; ++i)
    {
        acc += i;
    }

    // After reconvergence: all threads participate in wave sum
    int waveSum = WaveActiveSum(acc);

    outputBuffer[idx] = waveSum;
}

// All threads see the same wave sum = 50
// CHECK: 50
// CHECK: 50
// CHECK: 50
// CHECK: 50
