// Corner case: nested divergence with wave ops at each nesting level.
// Outer if splits threads into {0,2} vs {1,3}.
// Inner if further splits: thread 0 vs thread 2 (in the even branch).
// Wave ops at each level should see only the convergent thread set.
//
// Thread 0: outer-even, inner-low  → WaveActiveSum({0}) = 0,   then WaveActiveSum({0,2}) = 2
// Thread 1: outer-odd              → WaveActiveSum({1,3}) = 4
// Thread 2: outer-even, inner-high → WaveActiveSum({2}) = 2,   then WaveActiveSum({0,2}) = 2
// Thread 3: outer-odd              → WaveActiveSum({1,3}) = 4

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int result = 0;

    if ((idx & 1) == 0)
    {
        // Even threads: {0, 2}
        if (idx < 2)
        {
            // Only thread 0: WaveActiveSum of just {0}
            int innerSum = WaveActiveSum(idx);
            result = innerSum;  // thread 0 → 0
        }
        else
        {
            // Only thread 2: WaveActiveSum of just {2}
            int innerSum = WaveActiveSum(idx);
            result = innerSum;  // thread 2 → 2
        }
        // Reconverge even threads: {0, 2}
        int outerSum = WaveActiveSum(idx);
        result += outerSum * 100;  // 2 * 100 = 200
    }
    else
    {
        // Odd threads: {1, 3}
        int oddSum = WaveActiveSum(idx);
        result = oddSum + oddSum * 100;  // 4 + 400 = 404
    }

    outputBuffer[idx] = result;
}

// Thread 0: innerSum=0, outerSum=2 → 0 + 200 = 200
// Thread 1: oddSum=4 → 4 + 400 = 404
// Thread 2: innerSum=2, outerSum=2 → 2 + 200 = 202
// Thread 3: oddSum=4 → 4 + 400 = 404
// CHECK: 200
// CHECK: 404
// CHECK: 202
// CHECK: 404
