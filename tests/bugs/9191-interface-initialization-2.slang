// Additional test case for 9191-interface-initialization.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKWARN): -target hlsl -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKWARN): -target spirv -entry computeMain -stage compute -std 2025 -DEXPECT_ERROR=0
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

struct StructWithIfaceField2
/*CHECKERR:
       ^ candidate: StructWithIfaceField2.init(IHelper)
*/
{
    IHelper ifaceField;
/*CHECKWARN:
    ^^^^^^^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior
    ^^^^^^^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
*/
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    StructWithIfaceField2 tmp1 = { };
//CHECKERR:                      ^ not enough arguments to call
//CHECKERR:                      ^ not enough arguments to call (got 0, expected 1)
}
