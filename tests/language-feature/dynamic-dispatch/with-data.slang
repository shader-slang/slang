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
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3); // CHECK: 27
    outputBuffer[1] = f(1, 3); // CHECK: 30
}