// nested-switch-both-fallthrough.slang
//
// Test nested switches where both outer and inner switches have
// fallthrough behavior with wave operations. This triggers the
// switch-to-reconverged-switches lowering for both switches.
//
// Uses WaveActiveSum(1) to verify active thread counts.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Thread mapping: tid = outer*3 + inner
// Outer switch: case 0 (tid 0,1,2), case 1 (tid 0-5 due to fallthrough), default (tid 6,7,8)
// Inner switch: case 0 (tid 0,3), case 1 (tid 0,1,3,4), default (tid 2,5)
//
// Active thread counts:
// - outer case 0: 3 threads → WaveActiveSum(1) = 3, so mult = 7 + 3 = 10
// - inner case 0: 2 threads → WaveActiveSum(1) = 2
// - inner case 1: 4 threads → WaveActiveSum(1) = 4
//
// Expected values:
// tid=0: outer=0,inner=0: mult=10, inner: 2+4=6, result = 6*10 = 60
// tid=1: outer=0,inner=1: mult=10, inner: 4, result = 4*10 = 40
// tid=2: outer=0,inner=2: mult=10, inner: 100, result = 100*10 = 1000
// tid=3: outer=1,inner=0: mult=1, inner: 2+4=6, result = 6*1 = 6
// tid=4: outer=1,inner=1: mult=1, inner: 4, result = 4*1 = 4
// tid=5: outer=1,inner=2: mult=1, inner: 100, result = 100*1 = 100
// tid=6,7,8: outer=2,*: default -> result = -1

// Hex: 60=0x3C, 40=0x28, 1000=0x3E8, 6=0x6, 4=0x4, 100=0x64, -1=0xFFFFFFFF

// CHECK:      3C
// CHECK-NEXT: 28
// CHECK-NEXT: 3E8
// CHECK-NEXT: 6
// CHECK-NEXT: 4
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
        // 3 threads active (tid 0,1,2) → WaveActiveSum(1) = 3
        multiplier = 7 + WaveActiveSum(1);  // 7 + 3 = 10
        // Fall through to case 1

    case 1:
        // Inner switch with fallthrough and wave ops
        switch(inner)
        {
        case 0:
            // 2 threads active (tid 0,3) → WaveActiveSum(1) = 2
            result += WaveActiveSum(1);  // 2
            // Fall through to case 1

        case 1:
            // 4 threads active (tid 0,1,3,4) → WaveActiveSum(1) = 4
            result += WaveActiveSum(1);  // 4
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
