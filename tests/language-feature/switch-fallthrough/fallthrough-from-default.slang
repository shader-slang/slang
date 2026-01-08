// fallthrough-from-default.slang
//
// Test fall-through FROM the default case TO a subsequent case.
// This tests the pattern where default is not the last case.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      64
// CHECK-NEXT: C8
// CHECK-NEXT: 12C
// CHECK-NEXT: 32
// CHECK-NEXT: 1F4

//TEST_INPUT:ubuffer(data=[0 1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;
    int multiplier = 1;

    switch(val)
    {
    case 0:
        result = 100;
        break;
    case 1:
        result = 200;
        break;
    case 2:
        result = 300;
        break;
    default:
        // For unrecognized values, set multiplier and fall through to case 3
        multiplier = 10;
        // Fall through to case 3
    case 3:
        // When entering from default: multiplier = 10, result = 50 * 10 = 500
        // When entering directly at case 3: multiplier = 1, result = 50 * 1 = 50
        result = 50 * multiplier;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(5, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: case 0 -> result = 100
    // tid=1: case 1 -> result = 200
    // tid=2: case 2 -> result = 300
    // tid=3: case 3 directly -> multiplier = 1, result = 50 * 1 = 50
    //   Wait, but val=3 matches case 3, so it won't go through default!
    // tid=4: default (val=4 doesn't match any case) -> multiplier = 10,
    //        then falls through to case 3, result = 50 * 10 = 500
    outputBuffer[tid] = test(int(tid));
}
