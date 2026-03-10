//TEST:SIMPLE: -o tests/bugs/gh-10445.slang-module
//TEST:SIMPLE(filecheck=MODULE-CHECK): -target hlsl -stage compute -entry computeMain tests/bugs/gh-10445.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name outputBuffer

// Test for issue #10445: sizeof()/alignof()/countof() in static const
// definition crashes when compiling to a binary module.

// Verify round-trip through binary module produces correct constants.
// MODULE-CHECK: int(4)
// MODULE-CHECK: int(4)
// MODULE-CHECK: int(3)

RWStructuredBuffer<int> outputBuffer;

typedef uint MsgTy;
static const uint msgLen = sizeof(MsgTy);
static const uint msgAlign = alignof(MsgTy);

struct Dims<let each D : int>
{
    static const int dimCount = countof(D);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 4
    // CHECK-NEXT: 4
    // CHECK-NEXT: 3
    outputBuffer[0] = msgLen;
    outputBuffer[1] = msgAlign;
    outputBuffer[2] = Dims<1, 2, 3>.dimCount;
}
