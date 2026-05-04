// Functional test for wave operations on divergent if/else paths.
// From the docs: "wave-tangled functions apply only between the mutually
// convergent thread set by default."
//
// With 4 threads (indices 0,1,2,3):
//   Even threads (0,2) take the 'then' branch: WaveActiveMin over {0,2} = 0
//   Odd threads (1,3) take the 'else' branch: WaveActiveMin over {1,3} = 1
// After reconvergence, WaveActiveSum over all 4 results: 0+1+0+1 = 2

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int branchMin = 0;

    // Diverge: even vs odd threads
    if ((idx & 1) == 0)
    {
        // Even threads: min of {0, 2} = 0
        branchMin = WaveActiveMin(idx);
    }
    else
    {
        // Odd threads: min of {1, 3} = 1
        branchMin = WaveActiveMin(idx);
    }

    // After reconvergence: sum across all threads
    // branchMin values: {0, 1, 0, 1} → sum = 2
    int total = WaveActiveSum(branchMin);

    outputBuffer[idx] = total;
}

// All threads should see the same total = 2
// CHECK: 2
// CHECK: 2
// CHECK: 2
// CHECK: 2
