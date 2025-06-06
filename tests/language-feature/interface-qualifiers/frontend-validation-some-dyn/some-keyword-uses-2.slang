//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK: OpEntryPoint
// CHECK-NOT: error

interface IFoo
{
    int getVal();
    [mutating]
    void setVal(int v);
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
    [mutating]
    void setVal(int v)
    {
        a = v;
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
    [mutating]
    void setVal(int v)
    {
        a = v;
    }
};

void logic(inout some IFoo v1, out some IFoo v2)
{
    v1.setVal(1);
    v2 = StructForIFoo2(1);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = StructForIFoo1(1);
    some IFoo data2;
    logic(data1, data2);

    buffer[0] = data1.getVal() + data2.getVal();
}