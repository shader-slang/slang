//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK: error SOME_ID

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
        a = v+10;
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

// different returns, error

some IFoo returnIFooNested()
{
    if (output[0] == 0)
    {
        return StructForIFoo2(1);
    }
    else
    {
        return StructForIFoo2(2);
    }
}

some IFoo returnIFoo()
{
    if (output[1] == 0)
    {
        return StructForIFoo1(1);
    }
    else
    {
        return returnIFooNested();
    } 
}

RWStructuredBuffer<int> output;
void computeMain()
{
    some IFoo data3 = returnIFoo();
    output[0] = data3.getVal();
}