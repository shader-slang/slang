// conditional-break-fallthrough.slang
//
// Test a case that conditionally breaks or falls through based on
// a runtime value, with wave operations in the fallthrough path.
// This triggers the switch-to-reconverged-switches lowering.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Expected values (hex):
// tid=0: val=0 -> case 0 -> 100 = 0x64
// tid=1: val=1, break=true -> case 1 breaks -> 500 = 0x1F4
// tid=2: val=1, break=false -> case 1 falls through to default -> 200 = 0xC8
// tid=3: val=2 -> case 2 -> 200 = 0xC8
// tid=4: val=99 -> default directly -> 200 = 0xC8
// tid=5: val=100 -> default directly -> 200 = 0xC8

// CHECK:      64
// CHECK-NEXT: 1F4
// CHECK-NEXT: C8
// CHECK-NEXT: C8
// CHECK-NEXT: C8
// CHECK-NEXT: C8

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val, bool shouldBreak)
{
    int result = 0;

    switch(val)
    {
    case 0:
        // Wave op to trigger lowering (multiply by 0 to not affect value)
        result = 100 + int(WaveGetLaneIndex()) * 0;
        break;

    case 1:
        // Conditional break: if shouldBreak is true, break out
        // Otherwise fall through to default with wave op
        if (shouldBreak)
        {
            result = 500;
            break;
        }
        // Fall through to default

    default:
        // Wave op in fallthrough path
        result = 200 + int(WaveGetLaneIndex()) * 0;
        break;

    case 2:
        result = 200 + int(WaveGetLaneIndex()) * 0;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(6, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int val;
    bool shouldBreak;

    switch(tid)
    {
    case 0: val = 0; shouldBreak = false; break;   // case 0 -> 100
    case 1: val = 1; shouldBreak = true; break;    // case 1, break -> 500
    case 2: val = 1; shouldBreak = false; break;   // case 1, fallthrough -> default -> 200
    case 3: val = 2; shouldBreak = false; break;   // case 2 -> 200
    case 4: val = 99; shouldBreak = false; break;  // default -> 200
    default: val = 100; shouldBreak = false; break; // default -> 200
    }

    outputBuffer[tid] = test(val, shouldBreak);
}
