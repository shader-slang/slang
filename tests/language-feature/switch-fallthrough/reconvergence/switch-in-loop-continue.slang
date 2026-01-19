// switch-in-loop-continue.slang
//
// Test switch inside a loop where some cases use 'continue' to skip
// to the next loop iteration. The switch-to-if lowering wraps the switch
// in a do-while(false), so 'continue' must be correctly handled by
// eliminateMultiLevelBreak to target the outer loop.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu
// Note: DX11 (FXC) doesn't support 'continue' inside switch statements

// Expected values:
// tid=0: selector=0 in all iterations, always continue, counter never incremented -> 0
// tid=1: selector=1 in all iterations, always break from switch, counter += 1 each -> 3
// tid=2: selector=2 in all iterations, always break from switch, counter += 10 each -> 30
// tid=3: selector=3 (default) in all iterations, counter += 100 each -> 300

// Hex:
// 0 = 0x0
// 3 = 0x3
// 30 = 0x1E
// 300 = 0x12C

// CHECK:      0
// CHECK-NEXT: 3
// CHECK-NEXT: 1E
// CHECK-NEXT: 12C

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int selector)
{
    int counter = 0;

    for (int i = 0; i < 3; i++)
    {
        switch(selector)
        {
        case 0:
            // Skip the rest of the loop iteration
            continue;

        case 1:
            counter += 1;
            break;

        case 2:
            counter += 10;
            break;

        default:
            counter += 100;
            break;
        }

        // This code is reached after break (but not continue)
        // We don't add anything here to keep the test simple
    }

    return counter;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = test(int(tid));
}
