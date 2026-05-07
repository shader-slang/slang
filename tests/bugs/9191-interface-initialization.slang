// This test verifies that a warning is emitted if an interface
// variable is initialized with defaults.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKERR): -target hlsl -entry computeMain -stage compute -std 2026 -DEXPECT_ERROR=1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKERR): -target spirv -entry computeMain -stage compute -std 2026 -DEXPECT_ERROR=1


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

struct StructWithIfaceField
{
#if EXPECT_ERROR

    IHelper ifaceField = { };
//CHECKERR:              ^ cannot initialize an interface variable with defaults
//CHECKERR:              ^ initializing an interface variable with defaults is not supported. Please provide a compatible initializer or leave the variable uninitialized

#else

    IHelper ifaceField = { };
//CHECK:                 ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//CHECK:                 ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized

#endif
}

struct StructWithIfaceField2
{
    IHelper ifaceField;
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

StructWithIfaceField test2(int val)
{
    StructWithIfaceField ret = { };

    if (val == 0)
    {
        ret.ifaceField = HelperImpl1(val);
    }
    else
    {
        ret.ifaceField = HelperImpl2(val);
    }
    return ret;
}

StructWithIfaceField test3(int val)
{
#if EXPECT_ERROR
    StructWithIfaceField ret = { { } };
//CHECKERR:                      ^ cannot initialize an interface variable with defaults
//CHECKERR:                      ^ initializing an interface variable with defaults is not supported. Please provide a compatible initializer or leave the variable uninitialized
#else
    StructWithIfaceField ret = { { } };
//CHECK:                         ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//CHECK:                         ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
#endif

    if (val == 0)
    {
        ret.ifaceField = HelperImpl1(val);
    }
    else
    {
        ret.ifaceField = HelperImpl2(val);
    }
    return ret;
}

void test4()
{
    // note: this variant would print an error in an earlier compilation phase
    // (not enough arguments to call (got 0, expected 1)
    // StructWithIfaceField2 tmp1 = { };
    //
    // Tested separately in 9191-interface-initialization-2.slang


#if EXPECT_ERROR

    StructWithIfaceField2 tmp2 = { { } };
//CHECKERR:                        ^ cannot initialize an interface variable with defaults
//CHECKERR:                        ^ initializing an interface variable with defaults is not supported. Please provide a compatible initializer or leave the variable uninitialized

#else

    StructWithIfaceField2 tmp2 = { { } };
//CHECK:                           ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//CHECK:                           ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized

#endif
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
