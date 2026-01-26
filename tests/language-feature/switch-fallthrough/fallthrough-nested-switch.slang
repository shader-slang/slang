// fallthrough-nested-switch.slang
//
// Test fall-through behavior with nested switch statements.
// Variables modified in outer switch cases should be visible in inner switches
// and maintain their values through fall-through.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      B
// CHECK-NEXT: C
// CHECK-NEXT: A
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int outer, int inner)
{
    int result = 0;
    int x = 1;

    switch(outer)
    {
    case 0:
        x = x * 10;
        // Fall through
    case 1:
        // Nested switch
        switch(inner)
        {
        case 0:
            result = x + 1;
            break;
        case 1:
            result = x + 2;
            break;
        default:
            result = x;
            break;
        }
        break;

    default:
        result = -1;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(9, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Map tid to (outer, inner) pairs:
    // tid=0: outer=0, inner=0 -> x=10, result = 10+1 = 11
    // tid=1: outer=0, inner=1 -> x=10, result = 10+2 = 12
    // tid=2: outer=0, inner=2 -> x=10, result = 10
    // tid=3: outer=1, inner=0 -> x=1, result = 1+1 = 2
    // tid=4: outer=1, inner=1 -> x=1, result = 1+2 = 3
    // tid=5: outer=1, inner=2 -> x=1, result = 1
    // tid=6: outer=2, inner=0 -> result = -1 (default)
    // tid=7: outer=2, inner=1 -> result = -1 (default)
    // tid=8: outer=2, inner=2 -> result = -1 (default)
    int outer = int(tid) / 3;
    int inner = int(tid) % 3;
    outputBuffer[tid] = test(outer, inner);
}
