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
        // Even threads {0, 2}
        int outerSum = WaveActiveSum(idx); // 0+2=2
        if (idx == 0)
        {
            int innerSum = WaveActiveSum(idx); // just 0
            result = innerSum + outerSum * 100; // 0 + 200 = 200
        }
        else
        {
            int innerSum = WaveActiveSum(idx); // just 2
            result = innerSum + outerSum * 100; // 2 + 200 = 202
        }
    }
    else
    {
        // Odd threads {1, 3}
        int oddSum = WaveActiveSum(idx); // 1+3=4
        result = oddSum + oddSum * 100; // 4 + 400 = 404
    }
    outputBuffer[idx] = result;
}

// CHECK: 200
// CHECK: 404
// CHECK: 202
// CHECK: 404
