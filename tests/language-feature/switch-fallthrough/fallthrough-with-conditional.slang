// fallthrough-with-conditional.slang
//
// Test fall-through where the case body contains conditional logic.
// The variable's value at the fall-through point depends on which
// branch was taken.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      C8
// CHECK-NEXT: 190
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val, bool flag)
{
    int result = 0;
    int x = 1;

    switch(val)
    {
    case 0:
        if (flag)
            x = 100;
        else
            x = 200;
        // Fall through with x = 100 or 200 depending on flag
    case 1:
        result = x * 2;
        break;

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
    // Map tid to (val, flag) pairs:
    // tid=0: val=0, flag=true  -> x=100, result = 200
    // tid=1: val=0, flag=false -> x=200, result = 400
    // tid=2: val=1, flag=true  -> x=1, result = 2
    // tid=3: val=1, flag=false -> x=1, result = 2
    // tid=4-7: default -> result = -1
    int val = int(tid) / 2;
    bool flag = (tid % 2) == 0;
    outputBuffer[tid] = test(val, flag);
}
