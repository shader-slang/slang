//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// simple `some` copy to `some`

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

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = StructForIFoo1(1);
    some IFoo data2 = data1;
    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    buffer[0] = data2.getVal();
}