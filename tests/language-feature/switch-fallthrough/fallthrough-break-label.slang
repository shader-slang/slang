// fallthrough-break-label.slang
//
// Test that variables modified in fall-through cases are correctly
// accessible after the switch statement (at the break label).
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
    int x = 0;
    
    switch(val)
    {
    case 0:
        x = 10;
        // Fall through
    case 1:
        x = x + 5;
        break;
        
    case 2:
        x = 100;
        break;
        
    default:
        x = -1;
        break;
    }
    
    // x should be accessible here with correct value
    return x * 2;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: x = 10+5 = 15, result = 30
    // tid=1: x = 0+5 = 5, result = 10
    // tid=2: x = 100, result = 200
    // tid=3: x = -1, result = -2
    outputBuffer[tid] = test(int(tid));
}

