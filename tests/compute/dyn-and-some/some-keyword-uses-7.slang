//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// Struct as an argument to a `some` type function

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

int getSomeVal(some IFoo v)
{
    return v.getVal();
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    buffer[0] = getSomeVal(StructForIFoo1(1));
}