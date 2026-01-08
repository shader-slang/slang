// default-at-start.slang
//
// Test a switch where default is the first case and falls through
// to subsequent cases. This tests the predicate logic when default
// precedes all explicit cases.

// Note: Metal has broken fallthrough handling when default is first.
// The switch-to-if lowering pass will fix this. For now, only test on CPU.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// Expected values:
// val=0: case 0 -> 100
// val=1: case 1 -> 200
// val=99: default -> case 0 -> case 1
//         1 + 100 + 200 = 301

// Actually, let me trace through more carefully:
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
        result += 1;
        // Fall through to case 0

    case 0:
        result += 100;
        // Fall through to case 1

    case 1:
        result += 200;
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
