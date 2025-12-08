//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    This withOffset(float offset);
    float calc(float x);
}

struct A : IInterface
{
    float factor;
    A withOffset(float offset) { return {factor + offset}; }
    float calc(float x) { return factor * x * x * x; }
};

struct B : IInterface
{
    float factor1;
    float factor2;
    B withOffset(float offset) { return {factor1 + offset, factor2 + offset}; }
    float calc(float x) { return factor1 * x * x + factor2 * x; }
};

struct C : IInterface
{
    float factor;
    C withOffset(float offset) { return {factor + offset}; }
    float calc(float x) { return x; }
};

T transfer<T : IInterface>(T obj)
{
    return obj.withOffset(2.5f);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float calc<T : IInterface>(T obj, float y)
{
    return transfer(obj).calc(y);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A();
    else
        obj = B();

    return calc(obj, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 2); // CHECK: 20
    outputBuffer[1] = f(1, 2); // CHECK: 15
}