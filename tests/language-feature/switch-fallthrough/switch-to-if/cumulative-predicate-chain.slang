// cumulative-predicate-chain.slang
//
// Test cumulative predicate logic with multiple fallthrough chains
// and default in the middle. This matches the core transformation
// example in the switch-to-if lowering plan.

// Note: Metal and WGSL have broken fallthrough handling for this pattern.
// The switch-to-if lowering pass will fix this. For now, only test on CPU.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// Test the following switch structure:
// switch(selector) {
//   case 0:
//     first();
//     break;
//   case 4:
//     checkSomething() ? break : fallthrough
//   default:
//     something();
//     // fallthrough
//   case 1:
//   case 3:
//     somethingElse();
//     break;
//   case 2:
//     anotherThing();
//     break;
// }

// Expected values (decimal):
// val=0: case 0 -> first() -> 10
// val=1: case 1 -> somethingElse() -> 30
// val=2: case 2 -> anotherThing() -> 40
// val=3: case 3 -> somethingElse() -> 30
// val=4, cond=true: case 4 breaks -> 50
// val=4, cond=false: case 4 falls through -> default -> case 1/3 -> 20+30=50? No...
//   Actually: default sets result=20, then fallthrough to case 1/3 adds 30 -> 50
// val=5 (unmatched): default -> case 1/3 -> 20+30=50

// Let me recalculate more carefully:
// case 0: result = 10, break
// case 4 with break: result = 50, break
// case 4 without break: falls through to default, result += 20, falls to case 1/3, result += 30
//   So if we start at case 4 with result=0, we get 0 + 20 + 30 = 50
// default: result = 20, falls to case 1/3, result += 30 = 50
// case 1: result = 30, break
// case 3: result = 30, break
// case 2: result = 40, break

// Hex values:
// 10 = 0xA
// 30 = 0x1E
// 40 = 0x28
// 50 = 0x32

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
        result = 10;
        break;

    case 4:
        if (shouldBreakCase4)
        {
            result = 50;
            break;
        }
        // Fall through to default

    default:
        result += 20;
        // Fall through to case 1/3

    case 1:
    case 3:
        result += 30;
        break;

    case 2:
        result = 40;
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
    case 5: selector = 4; shouldBreakCase4 = false; break; // case 4, fall -> default -> 1/3 -> 50
    default: selector = 99; shouldBreakCase4 = false; break; // default -> 1/3 -> 50
    }

    outputBuffer[tid] = test(selector, shouldBreakCase4);
}
