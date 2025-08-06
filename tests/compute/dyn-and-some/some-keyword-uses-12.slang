//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// `some` copy to `some`, but through a cast-to-super

dyn interface IFoo
{
    int getVal();
}

dyn interface IFooOuter : IFoo
{
    int getValOuter();
}

struct StructForIFoo1 : IFooOuter
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
    int getValOuter()
    {
        return a;
    }
};

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // cast-to-super
    some IFoo data1 = StructForIFoo1(1);

    some IFooOuter data2 = StructForIFoo1(1);
    // cast-to-super
    some IFoo data3 = data2;

    // CHECK_SOME: OpEntryPoint
    // CHECK_SOME-NOT: error
    buffer[0] = data1.getVal() + data2.getValOuter() + data3.getVal();
}