// fallthrough-var-declared-in-case.slang
//
// Test fall-through where a variable is declared in one case and used
// in a fall-through target. This tests variable scoping with fall-through.
//
// NOTE: In C/C++, jumping over a variable initialization is undefined behavior.
// Slang should either:
// 1. Require variables used across fall-through to be declared before the switch
// 2. Or emit an error for this pattern
//
// This test documents the expected behavior.
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

    // Variable declared BEFORE switch - this pattern should work
    int x;

    switch(val)
    {
    case 0:
        x = 100;
        // Fall through
    case 1:
        // When entering from case 0: x = 100
        // When entering directly: x is uninitialized (undefined behavior in C)
        // Slang should ideally require initialization or emit a warning
        result = x;
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

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: x=100 from case 0, result = 100
    // tid=1: x is uninitialized - behavior depends on implementation
    // tid=2: result = 200
    // tid=3: result = -1
    outputBuffer[tid] = test(int(tid));
}
