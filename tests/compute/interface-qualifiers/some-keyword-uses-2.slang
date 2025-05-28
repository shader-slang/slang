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

__generic<T : IArithmetic>
interface IFooGeneric
{
    bool getVal();
}

__generic<T : IArithmetic>
struct StructForIFooGeneric : IFooGeneric<T>
{
    T v;
    __init(T v_in)
    {
        v = v_in;
    }

    bool getVal()
    {
        return v == T(0);
    }
}

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

    some IFoo good1;
    some IFooGeneric<float> good2;
    some IFooGeneric<float> good3 = StructForIFooGeneric<float>(1);

    buffer[0] = data1.getVal() 
                + data2.getVal()
                + StructForIFooGeneric<int>(2).getVal()
                + (int)good3.getVal()
                ;
}