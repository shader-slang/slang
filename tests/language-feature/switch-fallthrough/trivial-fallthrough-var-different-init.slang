// trivial-fallthrough-var-different-init.slang
//
// Test trivial fall-through (case grouping) where grouped cases need to
// pass different initial values to shared code. This tests the IRParam/IRVar
// lowering for case grouping.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// CHECK:      D2
// CHECK-NEXT: 69
// CHECK-NEXT: 23
// CHECK-NEXT: 7

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
    default:
        {
            // For case 0: compute 2 * 3 * 5 * 7 = 210
            // For default: compute 2 * 3 * 5 * 7 = 210
            int i = 2;
            i = i * 3;
            i = i * 5;
            result = i * 7;
        }
        break;
    case 1:
        {
            // compute 1 * 3 * 5 * 7 = 105
            int i = 1;
            i = i * 3;
            i = i * 5;
            result = i * 7;
        }
        break;
    case 2:
        {
            // compute 1 * 5 * 7 = 35
            int i = 1;
            i = i * 5;
            result = i * 7;
        }
        break;
    case 3:
        {
            // compute 1 * 7 = 7
            int i = 1;
            result = i * 7;
        }
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
    // tid=0: 210
    // tid=1: 105
    // tid=2: 35
    // tid=3: 7
    outputBuffer[tid] = test(int(tid));
}
