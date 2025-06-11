//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

interface IFoo<T>
{
    int getVal();
    [mutating]
    void setVal(int v);
}

struct StructForIFoo1<T> : IFoo<T>
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
some IFoo<T> nestedReturnLogic()
{
    return StructForIFoo1<T>(1);
}

// CHECK-NOT: error 33093
__generic<T>
some IFoo<T> logic(in some IFoo<T> v2, in some IFoo<T> v1)
{
    if(buffer[0] == 0)
    {
        return v2;
    }
    else
    {
        return v2; 
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo<int> data3 = StructForIFoo1<int>(4);
    some IFoo<int> data2 = StructForIFoo1<int>(2);
    some IFoo<int> data1 = logic<int>(data2, data3);
    buffer[0] = data1.getVal();
}