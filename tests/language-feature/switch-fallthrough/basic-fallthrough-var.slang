// basic-fallthrough-var.slang
//
// Test true fall-through where code executes in one case and then
// falls through to the next case. A variable modified in the first
// case should retain its value in the fall-through target.
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
    int result = 0;
    int x = 10;
    
    switch(val)
    {
    case 0:
        x = 100;
        // Fall through to case 1 (no break)
    case 1:
        // x should be 100 if we came from case 0, 10 if we entered at case 1
        result = x + val;
        break;
    case 2:
        result = 200;
        break;
    default:
        result = -1;
        break;
    }
    
    return result;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: x=100 (set in case 0), then case 1 body: result = 100 + 0 = 100
    // tid=1: x=10 (unchanged, entered at case 1): result = 10 + 1 = 11
    // tid=2: result = 200
    // tid=3: result = -1 (default)
    outputBuffer[tid] = test(int(tid));
}
