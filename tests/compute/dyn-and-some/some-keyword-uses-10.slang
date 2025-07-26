//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// `some` copy to `dyn` parameter

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

int getDynVal(dyn IFoo v)
{
    return v.getVal();
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = StructForIFoo1(1);

    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    buffer[0] = getDynVal(data1);
}