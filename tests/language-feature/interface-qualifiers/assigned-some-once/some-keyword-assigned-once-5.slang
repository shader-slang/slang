//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// loop without write
// if-else as write
// loop without write

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

RWBuffer<int> thing;
void logic(out some IFoo v1)
{
    // CHECK-NOT: error 41404
    for (int i = 0; i < thing[0]; i++)
    {
        thing[1] += 1;
    }

    if(buffer[0] == 0)
    {
        v1 = StructForIFoo1(1);
    }
    else if(buffer[0] == 1)
    {
        v1 = StructForIFoo1(2);
    }
    else
    {
        v1 = StructForIFoo1(3);
    }

    for (int i = 0; i < thing[0]; i++)
    {
        thing[2] += v1.getVal();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}