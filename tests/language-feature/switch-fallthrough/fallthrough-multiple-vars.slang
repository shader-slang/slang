// fallthrough-multiple-vars.slang
//
// Test fall-through with multiple variables being modified across cases.
// All variables should maintain their values correctly through fall-through.
//
// This is currently NOT supported in Slang but is planned for implementation.

// DISABLED until true fall-through is implemented
//DISABLE_TEST(compute):COMPARE_COMPUTE: -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE: -vk -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE: -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int a = 1;
    int b = 10;
    int c = 100;
    
    switch(val)
    {
    case 0:
        a = 2;
        b = 20;
        // c unchanged
        // Fall through
    case 1:
        // case 0: a=2, b=20, c=100
        // case 1: a=1, b=10, c=100
        c = c + a + b;
        // Fall through
    case 2:
        // case 0: a=2, b=20, c=122
        // case 1: a=1, b=10, c=111
        // case 2: a=1, b=10, c=100
        return a * 1000 + b * 10 + c % 1000;
        
    default:
        return -1;
    }
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: a=2, b=20, c=122 -> 2*1000 + 20*10 + 122 = 2322
    // tid=1: a=1, b=10, c=111 -> 1*1000 + 10*10 + 111 = 1211
    // tid=2: a=1, b=10, c=100 -> 1*1000 + 10*10 + 100 = 1200
    // tid=3: -1 (default)
    outputBuffer[tid] = test(int(tid));
}

