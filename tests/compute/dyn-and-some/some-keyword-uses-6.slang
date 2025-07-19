//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

dyn interface IFoo
{
    int getVal();
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
};

some IFoo makeSomeType(int v)
{
    return StructForIFoo1(v);
}

dyn IFoo makeDynType(int v)
{
    return StructForIFoo1(v);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // Check returning `some` from a function
    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    dyn IFoo data1;
    data1 = makeSomeType(1);

    dyn IFoo data2;
    data2 = makeDynType(1);

    buffer[0] = data1.getVal() + data2.getVal();
}