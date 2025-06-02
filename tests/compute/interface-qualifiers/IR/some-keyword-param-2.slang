//TEST:SIMPLE(filecheck=CHECK1): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DMISMATCH
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// TODO: check if this should error for both cases??
// CHECK1: error
// CHECK2: no idea
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

some IFoo nestedLogic()
{
#ifdef MISMATCH
    return StructForIFoo2(1);
#else
    return StructForIFoo1(1);
#endif
}
void logic(out some IFoo v1)
{
    // Not allowed to assign different types to `some` param
    if (buffer[0] == 0)
    {
        v1 = StructForIFoo1(1);
    }
    else
    {
        v1 = nestedLogic();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}