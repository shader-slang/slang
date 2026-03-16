//TEST:SIMPLE: -o tests/bugs/gh-10445.slang-module
//TEST:SIMPLE(filecheck=MODULE-CHECK): -target hlsl -stage compute -entry computeMain tests/bugs/gh-10445.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name outputBuffer

// Test for issue #10445: sizeof()/alignof()/countof() in static const
// definition crashes when compiling to a binary module.

// Verify round-trip through binary module produces correct constants.
// MODULE-CHECK: int(4)
// MODULE-CHECK: int(4)
// MODULE-CHECK: int(3)
// MODULE-CHECK: int(5)
// MODULE-CHECK: int(40)

RWStructuredBuffer<int> outputBuffer;

typedef uint MsgTy;
static const uint msgLen = sizeof(MsgTy);
static const uint msgAlign = alignof(MsgTy);
static const uint msgLenPlus1 = sizeof(MsgTy) + 1;
static const int lut[] = {10, 20, 30, 40};
static const int idxBySizeof = lut[sizeof(MsgTy) - 1];

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
    // CHECK-NEXT: 5
    // CHECK-NEXT: 40
    outputBuffer[0] = msgLen;
    outputBuffer[1] = msgAlign;
    outputBuffer[2] = Dims<1, 2, 3>.dimCount;
    outputBuffer[3] = msgLenPlus1;
    outputBuffer[4] = idxBySizeof;
}
