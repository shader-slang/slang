//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// Validate some uses of `some`.

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
};

void logic(inout some IFoo v1, in some IFoo v2)
{
    // Cannot assign `some` to `some`
    // CHECK_2026: ([[# @LINE+1]]): error 33088
    v1 = v2;
    some IFoo data4 = v2;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = StructForIFoo1(1);
    some IFoo data2 = StructForIFoo1(1);
    dyn IFoo data3 = StructForIFoo1(1);

    // Cannot assign `dyn` to `some`
    // CHECK_2026: ([[# @LINE+1]]): error 33087
    data1 = data3;
    logic(data1, data2);
    buffer[0] = data1.getVal() + data2.getVal();
}