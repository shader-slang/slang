//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK_SOME-COUNT-2: error
// not allowed copy `some` to `out dyn` parameter
// not allowed copy `some` to `inout dyn` parameter
//
// Note: we do not have/need special errors for `some` to
// `out dyn` and `some` to `inout dyn`

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

int setDynVal1<T>(out IFoo v) where T : IFoo
{
    v = StructForIFoo1(2);
}

int setDynVal2(inout IFoo v)
{
    v = StructForIFoo1(2);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;

    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    setDynVal1<IFoo>(data1);
    setDynVal2(data1);
}