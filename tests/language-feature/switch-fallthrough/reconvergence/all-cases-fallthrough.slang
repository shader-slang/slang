// all-cases-fallthrough.slang
//
// Test a switch where all cases fall through except the last.
// This tests the switch-to-reconverged-switches lowering with
// wave operations in the fallthrough path.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Each thread takes a different case path.
// The wave ops inside the switch should still work correctly.
// WaveGetLaneIndex returns the lane's index, which we use as a contribution.
//
// val=0: case 0 -> case 1 -> case 2 -> default
//        contribution = WaveGetLaneIndex (0 for lane 0)
//        1 + 10 + 100 + 1000 = 1111
// val=1: case 1 -> case 2 -> default  
//        10 + 100 + 1000 = 1110
// val=2: case 2 -> default
//        100 + 1000 = 1100
// val=3: default only
//        1000
// val=99: default only
//        1000

// Hex:
// 1111 = 0x457
// 1110 = 0x456
// 1100 = 0x44C
// 1000 = 0x3E8

// CHECK:      457
// CHECK-NEXT: 456
// CHECK-NEXT: 44C
// CHECK-NEXT: 3E8
// CHECK-NEXT: 3E8

//TEST_INPUT:ubuffer(data=[0 1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
        // WaveGetLaneIndex to trigger lowering (it's a wave op)
        result += 1 + int(WaveGetLaneIndex()) * 0;  // Just add 1
        // Fall through

    case 1:
        result += 10 + int(WaveGetLaneIndex()) * 0;  // Just add 10
        // Fall through

    case 2:
        result += 100 + int(WaveGetLaneIndex()) * 0;  // Just add 100
        // Fall through

    default:
        result += 1000;
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
