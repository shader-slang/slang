// loop-inside-switch.slang
//
// Test a switch where case bodies contain loops, with wave operations
// in the fallthrough path. This triggers the switch-to-reconverged-switches
// lowering and verifies loops are handled correctly.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Fallthrough group: cases 0,1. With stage guards:
// - case 0: 1 thread active, WaveActiveSum(1) = 1
// - case 1: 2 threads active, WaveActiveSum(1) = 2
//
// val=0: loop sums 1+2+3=6, WaveActiveSum=1, falls through, WaveActiveSum=2 -> 6+1+2=9
// val=1: WaveActiveSum=2 -> 2
// val=2: loop sums 1+2+3+4+5=15, WaveActiveSum=1 (only thread 2) -> 15+1=16
// val=99: default -> 1000

// Hex: 9=0x9, 2=0x2, 16=0x10, 1000=0x3E8

// CHECK:      9
// CHECK-NEXT: 2
// CHECK-NEXT: 10
// CHECK-NEXT: 3E8

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
        // Loop inside case, then fallthrough with wave op
        for (int i = 1; i <= 3; i++)
        {
            result += i;
        }
        result += WaveActiveSum(1);  // 1 thread active = 1
        // Fall through to case 1

    case 1:
        result += WaveActiveSum(1);  // 2 threads active = 2
        break;

    case 2:
        // Another loop, with break from switch (non-fallthrough case)
        for (int i = 1; i <= 5; i++)
        {
            result += i;
        }
        result += WaveActiveSum(1);  // 1 thread (thread 2 only) = 1
        break;

    default:
        result = 1000;
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
    case 2: val = 2; break;
    default: val = 99; break;
    }
    outputBuffer[tid] = test(val);
}
