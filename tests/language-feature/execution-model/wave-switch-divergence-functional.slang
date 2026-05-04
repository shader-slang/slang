// Functional test for switch statement divergence.
// From the docs: "Divergence occurs when threads jump to different case groups.
// Reconvergence occurs when threads exit the switch statement."
//
// 4 threads take different switch cases:
//   Thread 0: case 0 → val = 100
//   Thread 1: case 1 → val = 200
//   Thread 2: case 2 → val = 300
//   Thread 3: default → val = 400
//
// After reconvergence, WaveActiveSum = 100+200+300+400 = 1000

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int val = 0;

    // Each thread takes a different case
    switch (idx)
    {
    case 0:
        val = 100;
        break;
    case 1:
        val = 200;
        break;
    case 2:
        val = 300;
        break;
    default:
        val = 400;
        break;
    }

    // After switch reconvergence, all threads participate
    int total = WaveActiveSum(val);

    outputBuffer[idx] = total;
}

// All threads see the same sum = 1000
// CHECK: 1000
// CHECK: 1000
// CHECK: 1000
// CHECK: 1000
