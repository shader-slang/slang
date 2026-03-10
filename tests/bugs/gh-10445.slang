//TEST:SIMPLE: -o tests/bugs/gh-10445.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer

// Test for issue #10445: sizeof() in static const definition crashes
// when compiling to a binary module.

RWStructuredBuffer<int> outputBuffer;

typedef uint MsgTy;
static const uint msgLen = sizeof(MsgTy);

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 4
    outputBuffer[0] = msgLen;
}
