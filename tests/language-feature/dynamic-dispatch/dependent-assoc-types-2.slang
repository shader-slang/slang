//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):name=scratchBuffer
RWStructuredBuffer<uint> scratchBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IBufferRef
{
    uint[N] get<let N : int>(uint index);
    void set<let N : int>(uint index, uint[N] value);
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
        buffer.set<1>(0, bit_cast<uint[1]>(data));
    }
    static T deserialize(IBufferRef buffer)
    {
        return bit_cast<T>(buffer.get<1>(0));
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
        StandardSerializer<float>::serialize(data.y, buffer.withOffset(1));
    }

    static BData deserialize(IBufferRef buffer)
    {
        return BData(StandardSerializer<float>::deserialize(buffer.withOffset(0)),
                StandardSerializer<float>::deserialize(buffer.withOffset(1)));
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

ICalculation factoryAB(uint id)
{
    if (id == 0)
        return A();
    else
        return B();
}

struct BufferRef : IBufferRef
{
    uint offset;
    uint[N] get<let N : int>(uint index)
    {
        uint[N] result;
        for (int i = 0; i < N; ++i)
            result[i] = scratchBuffer[offset + i];
        return result;
    }

    void set<let N : int>(uint index, uint[N] value)
    {
        for (int i = 0; i < N; ++i)
            scratchBuffer[offset + i] = value[i];
    }

    This withOffset(uint offset)
    {
        return {this.offset + offset};
    }
};
IBufferRef getScratchBufferAsRef()
{
    return BufferRef(0);
}

void f(uint id, float q)
{
    let obj = factoryAB(id);
    obj.Data d = obj.make(q);
    IBufferRef buf = getScratchBufferAsRef();
    buf.set<1>(0, {(uint)id});
    obj.DataSerializer::serialize(d, buf.withOffset(1));
}

float g(float x)
{
    IBufferRef buf = getScratchBufferAsRef();
    uint id = buf.get<1>(0)[0];
    let obj = factoryAB(id);
    obj.Data d = obj.DataSerializer::deserialize(buf.withOffset(1));
    return obj.calc(d, x);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    f(0, 3.f);
    outputBuffer[0] = g(2.f);
    f(1, 2.f);
    outputBuffer[1] = g(2.f);

    // CHECK: 12.0
    // CHECK: 10.0
}