//TEST:SIMPLE(filecheck=CHECK): -target spirv -gnone -std 2026

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

extension<T : IInterface> T
{
    float calcTwice(float x)
    {
        return this.calc(x) + this.calc(x);
    }
}

IInterface factoryAB(uint id, float x)
{
    if (id == 0)
        return A();
    else
        return B();
}

float calcTwice(IInterface obj, float y)
{
    // CHECK: ([[# @LINE+1]]): error 33181
    return obj.calcTwice(y);
}

float f(uint id, float x)
{
    IInterface obj = factoryAB(id, x);
    return calcTwice(obj, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3);
    outputBuffer[1] = f(1, 3); 
}