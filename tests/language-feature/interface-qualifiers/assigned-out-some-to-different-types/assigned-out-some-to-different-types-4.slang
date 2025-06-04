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

some IFoo nestedReturnLogic()
{
    return StructForIFoo1(1);
}

// CHECK: error 33094
void logic(out some IFoo v1, in some IFoo v2)
{
    if(buffer[0] == 0)
    {
        v1 = StructForIFoo1(1);
    }
    else
    {
        v1 = nestedReturnLogic(); // error since all `some` are different type
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    some IFoo data2 = StructForIFoo1(2);
    logic(data1, data2);
    buffer[0] = data1.getVal();
}