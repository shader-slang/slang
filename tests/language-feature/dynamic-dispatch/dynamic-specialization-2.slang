//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    static float calc(float x);
}

struct A : IInterface
{
    static float calc(float x) { return x * x * x; }
};

struct B : IInterface
{
    static float calc(float x) { return x * x; }
};

struct C : IInterface
{
    static float calc(float x) { return x; }
};

struct D : IInterface
{
    static float calc(float x) { return x * x * x * x; }
};

struct E : IInterface
{
    static float calc(float x) { return x * x * x * x; }
};

float calc<T : IInterface>(T obj, float y)
{
    return T::calc(y);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A();
    else
        obj = B();

    return calc(obj, x); // Specialized for A & B.
}

float g(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = C();
    else
        obj = D();

    return calc(obj, x); // Specialized for C & D.
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 2); // CHECK: 8
    outputBuffer[1] = f(1, 2); // CHECK: 4

    outputBuffer[2] = g(0, 2); // CHECK: 2
    outputBuffer[3] = g(1, 2); // CHECK: 16
}