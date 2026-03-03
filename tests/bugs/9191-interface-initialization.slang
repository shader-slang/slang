// This test verifies that a warning is emitted if an interface
// variable is initialized with defaults.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

//Disabled due to issue 9462
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute


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

    //CHECK: result code = 0
    //CHECK: warning 30506

    IHelper ret = { };

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
