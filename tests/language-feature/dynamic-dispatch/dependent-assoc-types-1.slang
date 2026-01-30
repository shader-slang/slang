//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ISerializer<T>
{
    static void serialize(T data, RWStructuredBuffer<float> buffer);
    static T deserialize(RWStructuredBuffer<float> buffer);
}

interface ICalculation
{
    associatedtype Data;
    associatedtype DataSerializer : ISerializer<Data>;
    float calc(Data d, float x);
    Data make(float q);
}

struct StandardSerializer<T : __BuiltinFloatingPointType> : ISerializer<T>
{
    static void serialize(T data, RWStructuredBuffer<float> buffer)
    {
        // Note: just for testing.. don't ever serialize this way
        buffer[0] = __realCast<float>(data);
    }

    static T deserialize(RWStructuredBuffer<float> buffer)
    {
        return __realCast<T>(buffer[0]); 
    }
};

struct A : ICalculation
{
    typealias Data = float;
    typealias DataSerializer = StandardSerializer<float>;
    float calc(Data d, float x) { return d * x * x; }
    Data make(float q) { return q; }
};

struct BData 
{
    float x;
    float y;
};

struct BDataSerializer : ISerializer<BData>
{
    static void serialize(BData data, RWStructuredBuffer<float> buffer)
    {
        buffer[0] = data.x;
        buffer[1] = data.y;
    }

    static BData deserialize(RWStructuredBuffer<float> buffer)
    {
        return {buffer[0], buffer[1]};
    }
};

struct B : ICalculation
{
    typealias Data = BData;
    typealias DataSerializer = BDataSerializer;
    float calc(Data d, float x) { return d.x * x + d.y; }
    Data make(float q) { return {q, q * q}; }
};

struct C : ICalculation
{
    typealias Data = float;
    typealias DataSerializer = StandardSerializer<float>;
    float calc(Data d, float x) { return d * x; }
    Data make(float q) { return q; }
};

ICalculation factoryAB(uint id)
{
    if (id == 0)
        return A();
    else
        return B();
}

float f(uint id, float q)
{
    let obj = factoryAB(id);
    obj.Data d = obj.make(q);
    obj.DataSerializer::serialize(d, outputBuffer);
    outputBuffer[2] = id;
    return 0;
}

float g(float x)
{
    uint id = (uint)outputBuffer[2];
    let obj = factoryAB(id);
    obj.Data d = obj.DataSerializer::deserialize(outputBuffer);
    return obj.calc(d, x);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    f(0, 3.f);
    outputBuffer[3] = g(2.f);
    f(1, 2.f);
    outputBuffer[4] = g(2.f); 

    // Clear the first 3 elements
    outputBuffer[0] = 0.f;
    outputBuffer[1] = 0.f;
    outputBuffer[2] = 0.f;

    // CHECK: 0.0
    // CHECK: 0.0
    // CHECK: 0.0
    // CHECK: 12.0
    // CHECK: 8.0
}