// fallthrough-break-label.slang
//
// Test that variables modified in fall-through cases are correctly
// accessible after the switch statement (at the break label).

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      1E
// CHECK-NEXT: A
// CHECK-NEXT: C8
// CHECK-NEXT: FFFFFFFE

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int x = 0;

    switch(val)
    {
    case 0:
        x = 10;
        // Fall through
    case 1:
        x = x + 5;
        break;

    case 2:
        x = 100;
        break;

    default:
        x = -1;
        break;
    }

    // x should be accessible here with correct value
    return x * 2;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: x = 10+5 = 15, result = 30
    // tid=1: x = 0+5 = 5, result = 10
    // tid=2: x = 100, result = 200
    // tid=3: x = -1, result = -2
    outputBuffer[tid] = test(int(tid));
}
