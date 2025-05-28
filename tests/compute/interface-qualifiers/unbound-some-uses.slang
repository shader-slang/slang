//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK: OpEntryPoint
// CHECK-NOT: error

interface IFoo
{
    int getVal();
}

struct StructForIFoo1 : IFoo
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
struct StructForIFoo2 : IFoo
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
struct StructForIFoo3 : IFoo
{
    __init(int v)
    {
        a = v * 100;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }
};

// Complex Expr, so no support
//
// void test(Out<some IFoo> p)
// {

// }

void outParam(out some IFoo v1, out some IFoo v2)
{
    v1 = StructForIFoo1(1);
    v2 = StructForIFoo2(1);
}

some IFoo outReturn()
{
    return IFooStruct3(1);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    some IFoo data2;
    outParam(data1, data2);
    buffer[0] = data1.getVal() + data2.getVal() + outReturn().getVal();
}