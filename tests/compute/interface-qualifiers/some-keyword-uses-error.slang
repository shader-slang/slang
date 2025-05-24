//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

//CHECK-NOT: error

interface IFoo
{
    int getVal();
}

struct IFooStruct1 : IFoo
{
    __init(int v)
    {
        a = v;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }
};
struct IFooStruct2 : IFoo
{
    __init(int v)
    {
        a = v*10;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }
};

void logic(inout some IFoo v1, in some IFoo v2)
{
    v1 = v2;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = IFooStruct1(1);
    some IFoo data2 = IFooStruct1(1);
    dyn IFoo data3 = IFooStruct1(1);
    data1 = data3;
    logic(data1, data2);
    buffer[0] = data1.getVal() + data2.getVal();
}