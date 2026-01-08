// fallthrough-loop-interaction.slang
//
// Test interaction between fall-through and loops.
// Variables modified in loops should retain their final value
// when falling through to the next case.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      73
// CHECK-NEXT: 64
// CHECK-NEXT: C8
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;
    int sum = 0;

    switch(val)
    {
    case 0:
        // Sum 1+2+3+4+5 = 15
        for (int i = 1; i <= 5; i++)
            sum += i;
        // Fall through with sum = 15
    case 1:
        // When coming from case 0: sum = 15
        // When entering directly: sum = 0
        result = sum + 100;
        break;

    case 2:
        result = 200;
        break;

    default:
        result = -1;
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
    // tid=0: sum=15, result = 15 + 100 = 115
    // tid=1: sum=0, result = 0 + 100 = 100
    // tid=2: result = 200
    // tid=3: result = -1 (default)
    outputBuffer[tid] = test(int(tid));
}
