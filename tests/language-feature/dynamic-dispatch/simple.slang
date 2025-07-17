
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    float calc(float x);
}

struct A : IInterface
{
    float calc(float x) { return x * x * x; }
};

struct B : IInterface
{
    float calc(float x) { return x * x; }
};

struct C : IInterface
{
    float calc(float x) { return x; }
};

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A();
    else
        obj = B();

    return obj.calc(x);
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 1);
}