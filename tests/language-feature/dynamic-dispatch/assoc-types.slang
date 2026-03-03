//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ICalculation
{
    associatedtype Data;
    float calc(Data d, float x);
    Data make(float q);
}

struct A : ICalculation
{
    typealias Data = float;
    float calc(Data d, float x) { return d * x * x; }
    Data make(float q) { return q; }
};

struct BData 
{
    float x;
    float y;
};

struct B : ICalculation
{
    typealias Data = BData;
    float calc(Data d, float x) { return d.x * x * x + d.y; }
    Data make(float q) { return {q, q}; }
};

struct C : ICalculation
{
    typealias Data = float;
    float calc(Data d, float x) { return d * x; }
    Data make(float q) { return q; }
};

ICalculation factoryAB(uint id, float x)
{
    if (id == 0)
        return A();
    else
        return B();
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
    outputBuffer[0] = f(0, 1); // CHECK: 1.0
    outputBuffer[1] = f(1, 1); // CHECK: 2.0
}