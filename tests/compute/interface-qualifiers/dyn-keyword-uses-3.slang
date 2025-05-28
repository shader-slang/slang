//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// ensure `dyn T = some U` works

// CHECK: OpEntryPoint
// CHECK-NOT: error 

dyn interface DynInterface
{
    __init(int v_in);
    int getVal();
}

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
    dyn DynInterface dynVar = DynStruct1(1);

    some DynInterface someVar = DynStruct2(1);

    dynVar = someVar;

    buffer[0] = 0;
}