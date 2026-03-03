//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    associatedtype Extras;
    This withOffset(float offset);
    Extras preCalc(float x);
    float postCalc(Extras extras);
}

struct A : IInterface
{
    float factor;
    typealias Extras = float;
    A withOffset(float offset) { return {factor + offset}; }
    float preCalc(float x) { return factor * x * x * x; }
    float postCalc(float extras) { return extras; }
};

struct B : IInterface
{
    float factor1;
    float factor2;
    typealias Extras = float2;
    B withOffset(float offset) { return {factor1 + offset, factor2 + offset}; }
    float preCalc(float x) { return factor1 * x * x + factor2 * x; }
    float postCalc(Extras extras) { return dot(extras, float2(1, 1)); }
};

struct Foo<T : IInterface>
{
    T a;
    T.Extras extras;
}

Foo<T> make<T : IInterface>(T obj, float y)
{
    return {obj, obj.preCalc(y)};
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float calc<T : IInterface>(Foo<T> obj)
{
    return obj.a.postCalc(obj.extras);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float f(uint id, float x)
{
    IInterface obj;

    if (id == 0)
        obj = A(x);
    else
        obj = B(x, x + 1);

    return calc(make(obj, x));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 2); // CHECK: 16
    outputBuffer[1] = f(1, 2); // CHECK: 28
}