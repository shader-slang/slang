//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ICalculation<T : IFloat>
{
    associatedtype Data;
    T calc(Data d, T x);
    Data make(T q);
}

struct A<T : IFloat> : ICalculation<T>
{
    typealias Data = T;
    T calc(Data d, T x) { return d * x * x; }
    Data make(T q) { return q; }
};

struct BData<T : IFloat>
{
    T x;
    T y;
};

struct B<T : IFloat> : ICalculation<T>
{
    typealias Data = BData<T>;
    T calc(Data d, T x) { return d.x * x * x + d.y; }
    Data make(T q) { return {q, q}; }
};

struct C<T : IFloat> : ICalculation<T>
{
    typealias Data = T;
    T calc(Data d, T x) { return d * x; }
    Data make(T q) { return q; }
};

ICalculation<T> factoryAB<T : IFloat>(uint id, T x)
{
    if (id == 0)
        return A<T>();
    else
        return B<T>();
}

float f(uint id, float x)
{
    let obj = factoryAB(id, x);
    obj.Data d = obj.make(x);
    return obj.calc(d, x);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3); // CHECK: 27.0
    outputBuffer[1] = f(1, 3); // CHECK: 30.0
}