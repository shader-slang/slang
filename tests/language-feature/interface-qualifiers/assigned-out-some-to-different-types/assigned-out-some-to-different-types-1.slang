//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

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

// CHECK: error 33094
// CHECK: note: see use site
void logic(out some IFoo v1)
{
    if(buffer[0] == 0)
    {
        v1 = StructForIFoo1(1);
    }
    else
    {
        v1 = StructForIFoo2(3); // error, StructForIFoo1 != StructForIFoo2
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}