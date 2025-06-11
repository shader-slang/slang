//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// What this tests:
// Test returning 2 "equal" function. This is allowed
// since all functions must return the same value type.

interface IFoo
{
    int getVal();
    [mutating]
    void setVal(int v);
}
__generic<T>
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

__generic<T>
some IFoo nestedReturnLogic1()
{
    return StructForIFoo1<T>(1);
}

// CHECK-NOT: error 33093
some IFoo logic(in some IFoo v2)
{
    if (buffer[0] == 0)
    {
        return nestedReturnLogic1<int>();
    }
    else
    {
        return nestedReturnLogic1<int>();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data2 = StructForIFoo1<int>(2);
    some IFoo data1 = logic(data2);
    
    buffer[0] = data1.getVal();
}