// all-cases-fallthrough.slang
//
// Test a switch where all cases fall through except the last.
// This tests the switch-to-reconverged-switches lowering with
// wave operations in the fallthrough path.
//
// Uses WaveActiveSum to verify proper thread participation at each stage.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Each thread takes a different case path. With stage guards:
// - case 0: 1 thread active, WaveActiveSum(1) = 1
// - case 1: 2 threads active, WaveActiveSum(1) = 2
// - case 2: 3 threads active, WaveActiveSum(1) = 3
// - default: 5 threads active, WaveActiveSum(1) = 5
//
// val=0: case 0 -> case 1 -> case 2 -> default = 1 + 2 + 3 + 5 = 11
// val=1: case 1 -> case 2 -> default = 2 + 3 + 5 = 10
// val=2: case 2 -> default = 3 + 5 = 8
// val=3: default only = 5
// val=99: default only = 5

// Hex: 11=0xB, 10=0xA, 8=0x8, 5=0x5

// CHECK:      B
// CHECK-NEXT: A
// CHECK-NEXT: 8
// CHECK-NEXT: 5
// CHECK-NEXT: 5

//TEST_INPUT:ubuffer(data=[0 1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
        result += WaveActiveSum(1);  // 1 thread active = 1
        // Fall through

    case 1:
        result += WaveActiveSum(1);  // 2 threads active = 2
        // Fall through

    case 2:
        result += WaveActiveSum(1);  // 3 threads active = 3
        // Fall through

    default:
        result += WaveActiveSum(1);  // 5 threads active = 5
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(5, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int val;
    switch(tid)
    {
    case 0: val = 0; break;
    case 1: val = 1; break;
    case 2: val = 2; break;
    case 3: val = 3; break;
    default: val = 99; break;
    }
    outputBuffer[tid] = test(val);
}
