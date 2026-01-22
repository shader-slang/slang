// trivial-fallthrough-with-default.slang
//
// Test trivial fall-through where a case label shares block with default.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// CHECK:      64
// CHECK-NEXT: 3E7
// CHECK-NEXT: C8
// CHECK-NEXT: 3E7
// CHECK-NEXT: 3E7
// CHECK-NEXT: 3E7
// CHECK-NEXT: 3E7
// CHECK-NEXT: 3E7

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
        result = 100;
        break;
    case 1:
    default:
        // case 1 and default share this block
        result = 999;
        break;
    case 2:
        result = 200;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: 100
    // tid=1: 999
    // tid=2: 200
    // tid=3 and above: 999 (default)
    outputBuffer[tid] = test(int(tid));
}
