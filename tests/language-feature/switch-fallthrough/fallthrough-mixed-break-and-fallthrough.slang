// fallthrough-mixed-break-and-fallthrough.slang
//
// Test a switch with a mix of cases that break and cases that fall through.
// This tests that the compiler correctly handles both patterns in the same switch.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      6
// CHECK-NEXT: 3
// CHECK-NEXT: 64
// CHECK-NEXT: 23
// CHECK-NEXT: 7
// CHECK-NEXT: 1
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;
    int x = 1;

    switch(val)
    {
    case 0:
        x = x * 2;
        // Fall through
    case 1:
        x = x * 3;
        result = x;
        break;  // This case breaks

    case 2:
        result = 100;
        break;  // This case breaks

    case 3:
        x = x * 5;
        // Fall through
    case 4:
        x = x * 7;
        // Fall through
    case 5:
        result = x;
        break;  // Final case in chain breaks

    default:
        result = -1;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: x = 1*2*3 = 6, result = 6
    // tid=1: x = 1*3 = 3, result = 3
    // tid=2: result = 100
    // tid=3: x = 1*5*7 = 35, result = 35
    // tid=4: x = 1*7 = 7, result = 7
    // tid=5: x = 1 (unchanged), result = 1
    // tid=6: result = -1 (default)
    // tid=7: result = -1 (default)
    outputBuffer[tid] = test(int(tid));
}
