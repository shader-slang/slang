// cumulative-predicate-chain.slang
//
// Test complex fallthrough with default in the middle, with wave
// operations to trigger lowering.
//
// Uses WaveActiveSum(1) which returns the count of active threads.

// GPU backends only - wave ops not supported on CPU
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// Test the following switch structure:
// switch(selector) {
//   case 0: first(); break;
//   case 4: checkSomething() ? break : fallthrough
//   default: something(); fallthrough
//   case 1:
//   case 3: somethingElse(); break;
//   case 2: anotherThing(); break;
// }

// Active thread counts at each wave op:
// - case 0: 1 thread (tid=0) → WaveActiveSum(1) = 1
// - case 2: 1 thread (tid=2) → WaveActiveSum(1) = 1
// - default: 2 threads (tid=5,6) → WaveActiveSum(1) = 2
// - case 1/3: 4 threads (tid=1,3,5,6) → WaveActiveSum(1) = 4
//
// Expected values:
// tid=0: case 0 → 9 + 1 = 10
// tid=1: case 1/3 → 26 + 4 = 30
// tid=2: case 2 → 39 + 1 = 40
// tid=3: case 3 → 26 + 4 = 30
// tid=4: case 4 break → 50
// tid=5: case 4 fall → default → case 1/3 → (18+2) + (26+4) = 50
// tid=6: default → case 1/3 → (18+2) + (26+4) = 50

// Hex: 10=0xA, 30=0x1E, 40=0x28, 50=0x32

// CHECK:      A
// CHECK-NEXT: 1E
// CHECK-NEXT: 28
// CHECK-NEXT: 1E
// CHECK-NEXT: 32
// CHECK-NEXT: 32
// CHECK-NEXT: 32

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int selector, bool shouldBreakCase4)
{
    int result = 0;

    switch(selector)
    {
    case 0:
        // 1 thread active → WaveActiveSum(1) = 1
        result = 9 + WaveActiveSum(1);  // 9 + 1 = 10
        break;

    case 4:
        if (shouldBreakCase4)
        {
            result = 50;
            break;
        }
        // Fall through to default

    default:
        // 2 threads active (tid 5,6) → WaveActiveSum(1) = 2
        result += 18 + WaveActiveSum(1);  // 18 + 2 = 20
        // Fall through to case 1/3

    case 1:
    case 3:
        // 4 threads active (tid 1,3,5,6) → WaveActiveSum(1) = 4
        result += 26 + WaveActiveSum(1);  // 26 + 4 = 30
        break;

    case 2:
        // 1 thread active → WaveActiveSum(1) = 1
        result = 39 + WaveActiveSum(1);  // 39 + 1 = 40
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(7, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int selector;
    bool shouldBreakCase4;

    switch(tid)
    {
    case 0: selector = 0; shouldBreakCase4 = false; break; // case 0 -> 10
    case 1: selector = 1; shouldBreakCase4 = false; break; // case 1 -> 30
    case 2: selector = 2; shouldBreakCase4 = false; break; // case 2 -> 40
    case 3: selector = 3; shouldBreakCase4 = false; break; // case 3 -> 30
    case 4: selector = 4; shouldBreakCase4 = true; break;  // case 4, break -> 50
    case 5: selector = 4; shouldBreakCase4 = false; break; // case 4, fall -> 50
    default: selector = 99; shouldBreakCase4 = false; break; // default -> 50
    }

    outputBuffer[tid] = test(selector, shouldBreakCase4);
}
