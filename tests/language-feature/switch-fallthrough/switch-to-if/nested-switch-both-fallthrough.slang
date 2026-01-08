// nested-switch-both-fallthrough.slang
//
// Test nested switches where both outer and inner switches have
// fallthrough behavior. This verifies that the switch-to-if lowering
// correctly handles independent nested control flow.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// Outer switch:
// case 0: set multiplier=10, fallthrough
// case 1: run inner switch, break
// default: result = -1

// Inner switch:
// case 0: add 1, fallthrough
// case 1: add 2, break
// default: add 100, break

// Expected values:
// outer=0, inner=0: mult=10, inner: +1 fallthrough +2 = 3, result = 3*10 = 30
// outer=0, inner=1: mult=10, inner: +2 = 2, result = 2*10 = 20
// outer=0, inner=2: mult=10, inner: +100 = 100, result = 100*10 = 1000
// outer=1, inner=0: mult=1, inner: +1 fallthrough +2 = 3, result = 3*1 = 3
// outer=1, inner=1: mult=1, inner: +2 = 2, result = 2*1 = 2
// outer=1, inner=2: mult=1, inner: +100 = 100, result = 100*1 = 100
// outer=2, inner=*: default -> result = -1

// Hex:
// 30 = 0x1E
// 20 = 0x14
// 1000 = 0x3E8
// 3 = 0x3
// 2 = 0x2
// 100 = 0x64
// -1 = 0xFFFFFFFF

// CHECK:      1E
// CHECK-NEXT: 14
// CHECK-NEXT: 3E8
// CHECK-NEXT: 3
// CHECK-NEXT: 2
// CHECK-NEXT: 64
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int outer, int inner)
{
    int result = 0;
    int multiplier = 1;

    switch(outer)
    {
    case 0:
        multiplier = 10;
        // Fall through to case 1

    case 1:
        // Inner switch with fallthrough
        switch(inner)
        {
        case 0:
            result += 1;
            // Fall through to case 1

        case 1:
            result += 2;
            break;

        default:
            result += 100;
            break;
        }

        result *= multiplier;
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
    // tid=0: outer=0, inner=0
    // tid=1: outer=0, inner=1
    // tid=2: outer=0, inner=2
    // tid=3: outer=1, inner=0
    // tid=4: outer=1, inner=1
    // tid=5: outer=1, inner=2
    // tid=6: outer=2, inner=0
    // tid=7: outer=2, inner=1
    // tid=8: outer=2, inner=2
    int outer = int(tid) / 3;
    int inner = int(tid) % 3;
    outputBuffer[tid] = test(outer, inner);
}
