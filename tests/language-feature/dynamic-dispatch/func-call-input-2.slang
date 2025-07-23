
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

struct D : IInterface
{
    float calc(float x) { return x + x * x; }
};

struct E : IInterface
{
    float calc(float x) { return x * x + x * 3; }
};

IInterface factoryAB(uint id, float x)
{
    if (id == 0)
        return A();
    else
        return B();
}

IInterface factoryCD(uint id, float x)
{
    if (id == 0)
        return C();
    else
        return D();
}

// Should lower to accept A, B, C or D (but not E)
float calc(IInterface obj, float y)
{
    return obj.calc(y);
}

float f(uint id, float x)
{
    IInterface obj = factoryAB(id, x);
    return calc(obj, x);
}

float g(uint id, float x)
{
    IInterface obj = factoryCD(id, x);
    return calc(obj, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 1);
    outputBuffer[1] = g(0, 1);
}