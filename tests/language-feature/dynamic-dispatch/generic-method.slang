//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    T calc<T : __BuiltinFloatingPointType>(T x);
}

struct A : IInterface
{
    T calc<T : __BuiltinFloatingPointType>(T x) { return x * x * x; }
};

struct B : IInterface
{
    T calc<T : __BuiltinFloatingPointType>(T x) { return x * x; }
};

struct C : IInterface
{
    T calc<T : __BuiltinFloatingPointType>(T x) { return x; }
};

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A();
    else if (id == 1)
        obj = B();
    else
        obj = C();

    return obj.calc(x);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated specialized dynamic dispatch code{{.*}} 3 possible types:
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 2); // CHECK: 8
    outputBuffer[1] = f(1, 2); // CHECK: 4
}