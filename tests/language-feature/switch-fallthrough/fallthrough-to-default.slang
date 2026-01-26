// fallthrough-to-default.slang
//
// Test fall-through from a case to the default case.
//
// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      1F4
// CHECK-NEXT: 64
// CHECK-NEXT: C8
// CHECK-NEXT: 32

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;
    int multiplier = 1;

    switch(val)
    {
    case 0:
        multiplier = 10;
        // Fall through to default
    default:
        result = 50 * multiplier;
        break;
    case 1:
        result = 100;
        break;
    case 2:
        result = 200;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: multiplier=10, result = 50 * 10 = 500
    // tid=1: result = 100
    // tid=2: result = 200
    // tid=3: multiplier=1 (unchanged), result = 50 * 1 = 50 (default)
    outputBuffer[tid] = test(int(tid));
}
