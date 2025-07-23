
RWStructuredBuffer<uint8_t> outputBuffer;

interface IBufferRef
{
    uint8_t[N] get<let N : int>(uint index);
    void set<let N : int>(uint index, uint8_t[N] value);
    This withOffset(uint offset);
};

interface ISerializer<T>
{
    static void serialize(T data, IBufferRef buffer);
    static T deserialize(IBufferRef buffer);
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
    static void serialize(T data, IBufferRef buffer)
    {
        buffer.set<4>(0, bit_cast<uint8_t[4]>(data));
    }

    static T deserialize(IBufferRef buffer)
    {
        return bit_cast<T>(buffer.get<4>(0));
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
    static void serialize(BData data, IBufferRef buffer)
    {
        StandardSerializer<float>::serialize(data.x, buffer.withOffset(0));
        StandardSerializer<float>::serialize(data.y, buffer.withOffset(4));
    }

    static BData deserialize(IBufferRef buffer)
    {
        return BData(StandardSerializer<float>::deserialize(buffer.withOffset(0)),
                StandardSerializer<float>::deserialize(buffer.withOffset(4)));
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

struct BufferRef : IBufferRef
{
    uint offset;
    uint8_t[N] get<let N : int>(uint index)
    {
        uint offset = index * N;
        uint8_t[N] result;
        for (int i = 0; i < N; ++i)
        {
            result[i] = outputBuffer[offset + i];
        }
        return result;
    }

    void set<let N : int>(uint index, uint8_t[N] value)
    {
        uint offset = index * N;
        for (int i = 0; i < N; ++i)
        {
            outputBuffer[offset + i] = value[i];
        }
    }

    This withOffset(uint offset)
    {
        return {offset + this.offset};
    }
};
IBufferRef getOutputBufferAsRef()
{
    return BufferRef(0);
}

float f(uint id, float x)
{
    let obj = factoryAB(id, x);
    obj.Data d = obj.make(x);
    obj.DataSerializer::serialize(d, getOutputBufferAsRef());
}

float g(uint id, float x)
{
    let obj = factoryAB(id, x);
    obj.Data d = obj.DataSerializer::deserialize(getOutputBufferAsRef());
    return obj.calc(d, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    f(0, 1.f);
    g(0, 1.f);
}