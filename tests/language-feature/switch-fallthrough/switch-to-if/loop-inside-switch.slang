// loop-inside-switch.slang
//
// Test a switch where case bodies contain loops, with wave operations
// in the fallthrough path. This triggers the switch-to-reconverged-switches
// lowering and verifies loops are handled correctly.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Expected values:
// val=0: case 0 loop sums 1+2+3 = 6, falls through to case 1, adds 100 -> 106
// val=1: case 1 adds 100 -> 100
// val=2: case 2 loop sums 1+2+3+4+5 = 15, breaks -> 15
// val=99: default -> 1000

// Hex:
// 106 = 0x6A
// 100 = 0x64
// 15 = 0xF
// 1000 = 0x3E8

// CHECK:      6A
// CHECK-NEXT: 64
// CHECK-NEXT: F
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
        // Wave op to trigger lowering (multiply by 0)
        result += int(WaveGetLaneIndex()) * 0;
        // Fall through to case 1

    case 1:
        result += 100 + int(WaveGetLaneIndex()) * 0;
        break;

    case 2:
        // Another loop, with break from switch
        for (int i = 1; i <= 5; i++)
        {
            result += i;
        }
        result += int(WaveGetLaneIndex()) * 0;
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
