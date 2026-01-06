// trivial-fallthrough-case-grouping.slang
//
// Test "trivial fall-through" where multiple case labels share the same block.
// This is currently supported in Slang.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj

// CHECK:      64
// CHECK-NEXT: 65
// CHECK-NEXT: CA
// CHECK-NEXT: CB
// CHECK-NEXT: CC
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;

    switch(val)
    {
    case 0:
    case 1:
        // Both case 0 and case 1 execute this block
        result = 100 + val;
        break;
    case 2:
    case 3:
    case 4:
        // Cases 2, 3, 4 share this block
        result = 200 + val;
        break;
    default:
        result = -1;
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
    // tid=0: 100 + 0 = 100
    // tid=1: 100 + 1 = 101
    // tid=2: 200 + 2 = 202
    // tid=3: 200 + 3 = 203
    // tid=4: 200 + 4 = 204
    // tid=5: -1
    // tid=6: -1
    // tid=7: -1
    outputBuffer[tid] = test(int(tid));
}
