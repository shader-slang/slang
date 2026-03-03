//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float calc(float x);
}

interface IFactory
{
    IFoo create(float q);
}

struct AFoo : IFoo
{
    float q;
    float calc(float x) { return q * x; }
};

struct A : IFactory
{
    IFoo create(float q) { return AFoo(q); }
};

struct B : IFactory, IFoo
{
    float q;
    float calc(float x) { return q * x * x; }
    IFoo create(float q) { return This(q); }
};

struct CFoo : IFoo
{
    float calc(float x) { return x; }
};

struct C : IFactory
{
    IFoo create(float q) { return CFoo(); }
};

IFactory getFactory(uint id, float x)
{
    if (id == 0)
        return A();
    else
        return B(x);
}

float calc(IFoo obj, float y)
{
    return obj.calc(y);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float f(uint id, float x)
{
    IFactory factory = getFactory(id, x);
    IFoo foo = factory.create(2 * x);
    return calc(foo, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 2); // CHECK: 8
    outputBuffer[1] = f(1, 2); // CHECK: 16
}