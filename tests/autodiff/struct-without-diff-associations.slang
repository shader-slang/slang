//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -line-directive-mode none -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -line-directive-mode none -entry computeMain -stage compute
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Foo2 : IDifferentiable
{
    float dx[3];
}

struct Foo : IDifferentiable
{
    typealias Differential = Foo2;

    float x[3];
//CHECK:  ^ missing field in differential type
//CHECK:  ^ differentiable member 'x' should have a corresponding field in 'Foo2'
};

[Differentiable]
Foo getFoo(float x)
{
    return { { x, x, x } };
}

[Differentiable]
float foobar(float x)
{
    int i = 3 * int(floor(x));
    Foo foo = getFoo(x);
    return foo.x[i] * foo.x[i];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        float a = 0.5;
        var d = fwd_diff(foobar)(diffPair(a, 1.0)).d;
        outputBuffer[0] = d;
    }
}
