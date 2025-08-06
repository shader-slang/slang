//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// test some/dyn as an output parameter

// CHECK: OpEntryPoint
// CHECK-NOT: error

dyn interface IFooDyn
{
    int getVal();
}

struct StructForIFooDyn : IFooDyn
{
    int data = 0;
    __init(int dataIn = 1)
    {
        data = dataIn;
    }
    int getVal()
    {
        return data;
    }
}

interface IFooSome
{
    int getVal();
    [mutating]
    void setVal(int v);
}

interface IFooSomeOuter : IFooSome
{
}

struct StructForIFooSome1 : IFooSomeOuter
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
struct StructForIFooSome2 : IFooSomeOuter
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

struct StructForIFooSome3 : IFooSomeOuter
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
}

void logic(out dyn IFooDyn v1, out some IFooSome v2)
{
    v1 = StructForIFooDyn(1);
    v2 = StructForIFooSome2(1);
    
    v2.setVal(1 + v1.getVal());
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    dyn IFooDyn data1;
    some IFooSome data2;
    logic(data1, data2);

    buffer[0] = data1.getVal() + data2.getVal();
}