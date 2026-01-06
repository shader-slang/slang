// fallthrough-chain.slang
//
// Test a chain of fall-through cases where a variable is progressively
// modified as execution falls through multiple cases.
//
// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      D2
// CHECK-NEXT: 69
// CHECK-NEXT: 23
// CHECK-NEXT: 7
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 1;

    switch(val)
    {
    case 0:
        result = result * 2;
        // Fall through
    case 1:
        result = result * 3;
        // Fall through
    case 2:
        result = result * 5;
        // Fall through
    case 3:
        result = result * 7;
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(5, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results (Duff's device style accumulation):
    // tid=0: 1 * 2 * 3 * 5 * 7 = 210
    // tid=1: 1 * 3 * 5 * 7 = 105
    // tid=2: 1 * 5 * 7 = 35
    // tid=3: 1 * 7 = 7
    // tid=4: -1 (default)
    outputBuffer[tid] = test(int(tid));
}
