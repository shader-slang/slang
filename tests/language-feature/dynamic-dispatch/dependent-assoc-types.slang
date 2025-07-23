
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
    float calc(Data d, float x) { return d.x * x * x + d.y; }
    Data make(float q) { return {q, q}; }
};

struct C : ICalculation
{
    typealias Data = float;
    typealias DataSerializer = StandardSerializer<float>;
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
    obj.DataSerializer::serialize(d, outputBuffer);
    outputBuffer[3] = id;
}

float g(uint id, float x)
{
    let obj = factoryAB(id, x);
    obj.Data d = obj.DataSerializer::deserialize(outputBuffer);
    return obj.calc(d, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    f(0, 1.f);
    g(0, 1.f);
}