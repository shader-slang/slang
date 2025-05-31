//IGNORE:TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

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

some IFoo returnIFoo(in some data1, in some data2)
{
    // TODO in IR -- error, returning multiple different types, invalid
    if (data1.getVal() == 0)
    {
        return data1;
    }
    else
    {
        return data2;
    } 
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = StructForIFoo1();
    some IFoo data2 = StructForIFoo1();
    returnIFoo(data1, data2);
    buffer[0] = data1.getVal();
}