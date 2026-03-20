// Verify that an explicit [anyValueSize(7)] produces an error when a
// conforming type requires 8 bytes of storage. LargeImpl has two int
// fields (8 bytes total), which exceeds the 7-byte limit by exactly 1
// byte — boundary value testing for off-by-one in the size check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[anyValueSize(7)]
interface IFoo
{
    int getValue();
}

struct SmallImpl : IFoo
{
    int v;
    int getValue() { return v; }
}

struct LargeImpl : IFoo
/*CHECK:
       ^^^^^^^^^ type does not fit in size required by interface
       ^^^^^^^^^ type 'LargeImpl' does not fit in the size required by its conforming interface.
       ^^^^^^^^^ sizeof(LargeImpl) is 8, limit is 7
*/
{
    int a;
    int b;
    int getValue() { return a + b; }
}

RWStructuredBuffer<int> outputBuffer;

IFoo makeFoo(int id)
{
    if (id == 0)
    {
        SmallImpl s;
        s.v = 1;
        return s;
    }
    else
    {
        LargeImpl l;
        l.a = 2;
        l.b = 3;
        return l;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var f = makeFoo(0);
    outputBuffer[0] = f.getValue();
}
