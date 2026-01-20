// default-at-start.slang
//
// Test a switch where default is the first case and falls through
// to subsequent cases, with WaveActiveSum to verify thread counts.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// 4 threads with val: 0, 1, 99, 99
// Fallthrough group: default -> case 0 -> case 1
// - default stage: threads 2,3 active, WaveActiveSum(1) = 2
// - case 0 stage: threads 0,2,3 active, WaveActiveSum(1) = 3
// - case 1 stage: all 4 threads active, WaveActiveSum(1) = 4
//
// val=0 (thread 0): case 0 + case 1 = 3 + 4 = 7
// val=1 (thread 1): case 1 only = 4
// val=99 (threads 2,3): default + case 0 + case 1 = 2 + 3 + 4 = 9

// Hex: 7=0x7, 4=0x4, 9=0x9

// CHECK:      7
// CHECK-NEXT: 4
// CHECK-NEXT: 9
// CHECK-NEXT: 9

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    default:
        result += WaveActiveSum(1);  // 2 threads (2,3) active
        // Fall through to case 0

    case 0:
        result += WaveActiveSum(1);  // 3 threads (0,2,3) active
        // Fall through to case 1

    case 1:
        result += WaveActiveSum(1);  // 4 threads active
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int val;
    switch(tid)
    {
    case 0: val = 0; break;
    case 1: val = 1; break;
    default: val = 99; break;
    }
    outputBuffer[tid] = test(val);
}
