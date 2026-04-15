// This test verifies that a warning is emitted if an interface
// variable is initialized with defaults.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKERR): -target hlsl -entry computeMain -stage compute -std 2026 -DEXPECT_ERROR=1


interface IHelper
{
    int getVal();
}

struct HelperImpl1 : IHelper
{
    int storedVal;

    int getVal() { return storedVal; }
}

struct HelperImpl2 : IHelper
{
    int storedVal;

    int getVal() { return storedVal + 1; }
}

struct HelperImpl3 : IHelper
{
    int storedVal;

    int getVal() { return storedVal + 2; }
}

IHelper test(int val)
{
    // check for the presense of the warning

#if EXPECT_ERROR
    IHelper ret = { };
//CHECKERR:       ^ cannot initialize an interface variable with defaults
//CHECKERR:       ^ initializing an interface variable with defaults is not supported. Please provide a compatible initializer or leave the variable uninitialized

#else
    IHelper ret = { };
//CHECK:          ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//CHECK:          ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
#endif

    switch (val % 4)
    {
    case 0:
    {
        IHelper ret2 = { HelperImpl1(val) };
        ret = ret2;
        break;
    }

    case 1:
    {
        IHelper ret2 = { HelperImpl2(val) };
        ret = ret2;
        break;
    }

    case 2:
    {
        IHelper ret2 = { HelperImpl3(val) };
        ret = ret2;
        break;
    }

    default:
        break;
    }

    return ret;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    IHelper helper = test(int(tid));
    int outputVal = helper.getVal();
    gOutputBuffer[tid] = outputVal;
}
