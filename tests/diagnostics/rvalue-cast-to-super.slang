//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// An explicit conversion from `IFooOuter` to `IFoo` produces an r-value. The two calls below use
// the same conversion outside and inside a lambda so that lambda-capture rewriting cannot
// accidentally make the converted expression writable.

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
    IFooOuter outerData;

    // CHECK: [E30047]: argument must be l-value
    logic(IFoo(outerData));

    let testLambdaLocal = () =>
    {
        IFooOuter lambdaData;

        // CHECK: [E30047]: argument must be l-value
        logic(IFoo(lambdaData));
    };
    testLambdaLocal();
}
