// default-at-start.slang
//
// Test a switch where default is the first case and falls through
// to subsequent cases, with wave operations to trigger lowering.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// switch structure:
//   default: result += 1; fallthrough
//   case 0:  result += 100; fallthrough
//   case 1:  result += 200; break

// val=0: enters at case 0, result = 0 + 100 + 200 = 300
// val=1: enters at case 1, result = 0 + 200 = 200
// val=99: enters at default, result = 0 + 1 + 100 + 200 = 301

// Hex:
// 300 = 0x12C
// 200 = 0xC8
// 301 = 0x12D

// CHECK:      12C
// CHECK-NEXT: C8
// CHECK-NEXT: 12D
// CHECK-NEXT: 12D

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    default:
        // Wave op to trigger lowering
        result += 1 + int(WaveGetLaneIndex()) * 0;
        // Fall through to case 0

    case 0:
        result += 100 + int(WaveGetLaneIndex()) * 0;
        // Fall through to case 1

    case 1:
        result += 200 + int(WaveGetLaneIndex()) * 0;
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
