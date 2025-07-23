
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    float calc(float x);
}

struct A : IInterface
{
    float factor;
    float calc(float x) { return x * x * factor; }
};

struct B : IInterface
{
    float factor;
    float offset;
    float calc(float x) { return x * x * factor + offset; }
};

struct C : IInterface
{
    float calc(float x) { return x; }
};

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A(x);
    else
        obj = B(x, x);

    return obj.calc(x);
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 1);
}