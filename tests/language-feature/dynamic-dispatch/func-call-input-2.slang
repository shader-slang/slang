//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

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
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 4 possible types:
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
    outputBuffer[0] = f(0, 3); // CHECK: 27
    outputBuffer[1] = f(1, 3); // CHECK: 9
    outputBuffer[2] = g(0, 3); // CHECK: 3
    outputBuffer[3] = g(1, 3); // CHECK: 12
}