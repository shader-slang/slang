//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// ensure visitSomeTypeExpr does not segfault with bad definition
// and gets far enough to lint errors.

// CHECK: undefined identifier

struct DynStruct1 : DynInterface
{
    int v;
    
    __init(int v_in)
    {
        v = v_in;
    }

    int getVal()
    {
        return v;
    }
}

struct DynStruct2 : DynInterface
{
    float v;

    __init(int v_in)
    {
        v = (float)v_in;
    }

    int getVal()
    {
        return (int)v;
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    dyn DynStruct dynVar = DynStruct1();

    some DynStruct someVar = DynStruct2();

    dynVar = someVar;

    buffer[0] = 0;
}