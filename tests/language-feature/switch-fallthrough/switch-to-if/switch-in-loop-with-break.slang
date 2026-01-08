// switch-in-loop-with-break.slang
//
// Test switch inside a loop where some cases use 'break' to exit just
// the switch, while the loop continues. Verify that the synthetic
// do-while(false) wrapper correctly distinguishes switch-break from loop-break.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// Expected values:
// tid=0: selector=0, case 0 adds 1 each iteration, post-switch adds 10 each
//        3 iterations: (1+10)*3 = 33
// tid=1: selector=1, case 1 adds 5 each iteration, post-switch adds 10 each
//        3 iterations: (5+10)*3 = 45
// tid=2: selector=2, case 2 has inner if that breaks loop on i==1
//        i=0: adds 100, post adds 10 -> 110
//        i=1: enters case 2, breaks out of LOOP -> stops
//        result = 110
// tid=3: selector=3 (default), adds 1000 each, post adds 10 each
//        3 iterations: (1000+10)*3 = 3030

// Hex:
// 33 = 0x21
// 45 = 0x2D
// 110 = 0x6E
// 3030 = 0xBD6

// CHECK:      21
// CHECK-NEXT: 2D
// CHECK-NEXT: 6E
// CHECK-NEXT: BD6

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int selector)
{
    int result = 0;

    for (int i = 0; i < 3; i++)
    {
        switch(selector)
        {
        case 0:
            result += 1;
            break;  // break from switch, not loop

        case 1:
            result += 5;
            break;  // break from switch, not loop

        case 2:
            // Conditionally break from the LOOP (not just switch)
            // This uses a labeled break pattern, but since we can't use labels,
            // we'll use a flag and break both.
            if (i == 1)
            {
                // We want to break from the loop here.
                // We'll use a return for simplicity since this tests switch-break.
                return result;
            }
            result += 100;
            break;

        default:
            result += 1000;
            break;
        }

        // This is reached after every switch break (except case 2 return)
        result += 10;
    }

    return result;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = test(int(tid));
}
