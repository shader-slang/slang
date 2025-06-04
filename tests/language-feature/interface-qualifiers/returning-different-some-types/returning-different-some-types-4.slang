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

some IFoo nestedReturnLogic()
{
    return StructForIFoo1(1);
}

// CHECK: error 33093
some IFoo logic(in some IFoo v2)
{
    if(buffer[0] == 0)
    {
        return StructForIFoo1(1);
    }
    else
    {
        return nestedReturnLogic();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data2 = StructForIFoo1(2);
    some IFoo data1 = logic(data2);
    buffer[0] = data1.getVal();
}