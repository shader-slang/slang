//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface IFoo
{
    int getVal();
    [mutating]
    void setVal(int v);
}

interface IFooOuter : IFoo
{
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
    [mutating]
    void setVal(int v)
    {
        a = v;
    }
};

void logic(inout IFoo v)
{
    v = StructForIFoo1(1);
}
RWStructuredBuffer<int> buffer;

[shader("compute")]
void computeMain()
{
    IFooOuter data1;

    // CHECK: [E30047]: argument must be l-value
    // CHECK: rvalue-cast-to-super.slang:[[# @LINE+1]]:
    logic(data1);
}
