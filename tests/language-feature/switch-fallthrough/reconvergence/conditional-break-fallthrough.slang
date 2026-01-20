// conditional-break-fallthrough.slang
//
// Test a case with multiple conditional breaks and result manipulation
// in between, with wave operations in the fallthrough path.
// This triggers the switch-to-reconverged-switches lowering.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Expected values (hex):
// tid=0: val=0 -> case 0 -> 100 = 0x64
// tid=1: val=1, break1=true -> case 1 breaks at first if -> 500 = 0x1F4
// tid=2: val=1, break1=false, break2=true -> case 1 breaks at second if -> 0+10+600 = 610 = 0x262
// tid=3: val=1, break1=false, break2=false -> case 1 falls through -> 0+10+20 + default(200) = 230 = 0xE6
// tid=4: val=2 -> case 2 -> 200 = 0xC8
// tid=5: val=99 -> default directly -> 200 = 0xC8
// tid=6: val=100 -> default directly -> 200 = 0xC8

// CHECK:      64
// CHECK-NEXT: 1F4
// CHECK-NEXT: 262
// CHECK-NEXT: E6
// CHECK-NEXT: C8
// CHECK-NEXT: C8
// CHECK-NEXT: C8

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val, bool shouldBreak1, bool shouldBreak2)
{
    int result = 0;

    switch(val)
    {
    case 0:
        // Wave op to trigger lowering (multiply by 0 to not affect value)
        result = 100 + int(WaveGetLaneIndex()) * 0;
        break;

    case 1:
        // First conditional break
        if (shouldBreak1)
        {
            result = 500;
            break;
        }
        // Manipulate result after first conditional
        result += 10;

        // Second conditional break
        if (shouldBreak2)
        {
            result += 600;
            break;
        }
        // Manipulate result after second conditional
        result += 20;
        // Fall through to default

    default:
        // Wave op in fallthrough path
        result += 200 + int(WaveGetLaneIndex()) * 0;
        break;

    case 2:
        result = 200 + int(WaveGetLaneIndex()) * 0;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(7, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int val;
    bool shouldBreak1;
    bool shouldBreak2;

    switch(tid)
    {
    case 0: val = 0; shouldBreak1 = false; shouldBreak2 = false; break;  // case 0 -> 100
    case 1: val = 1; shouldBreak1 = true;  shouldBreak2 = false; break;  // case 1, break1 -> 500
    case 2: val = 1; shouldBreak1 = false; shouldBreak2 = true;  break;  // case 1, break2 -> 610
    case 3: val = 1; shouldBreak1 = false; shouldBreak2 = false; break;  // case 1, fallthrough -> 230
    case 4: val = 2; shouldBreak1 = false; shouldBreak2 = false; break;  // case 2 -> 200
    case 5: val = 99; shouldBreak1 = false; shouldBreak2 = false; break; // default -> 200
    default: val = 100; shouldBreak1 = false; shouldBreak2 = false; break; // default -> 200
    }

    outputBuffer[tid] = test(val, shouldBreak1, shouldBreak2);
}
