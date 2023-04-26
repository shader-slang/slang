//TEST_IGNORE_FILE:
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct DataBlob
{
    float data[10];
};

struct DataBlock : IDifferentiable
{
    uint classID;
    DataBlob data;

    typedef DataBlock Differential;

    [BackwardDifferentiable]
    static Differential dadd(Differential a, Differential b)
    {
        // Create a dynamic object using the id, and use convert to cast it to the correct type
        // before accumulating, and the reinterpret it back.

        var obj = createDynamicObject<IInterface>(a.classID, a.data);
        return obj.toDifferentialDataBlob(
            obj.ThisData.dadd(
                obj.toDifferentialData(a),
                obj.toDifferentialData(b)));
    }
};

struct AData : IDifferentiable
{
    float a;
    float b;
};

struct BData : IDifferentiable
{
    float a;
    int b;
};

interface IInterface
{
    associatedtype ThisData : IDifferentiable;

    [BackwardDifferentiable]
    float compute(DataBlock data, float c);

    [BackwardDifferentiable]
    static ThisData toData(DataBlock data);

    [BackwardDifferentiable]
    static ThisData.Differential toDifferentialData(DataBlock.Differential data);

    [BackwardDifferentiable]
    static DataBlock toDataBlob(ThisData data);

    [BackwardDifferentiable]
    static DataBlock.Differential toDifferentialDataBlob(ThisData.Differential data);
};

struct A : IInterface
{
    typealias ThisData = AData;

    [BackwardDifferentiable]
    float compute(DataBlock data, float c)
    {
        AData aData = toData(data);
        return aData.a + aData.b * c;
    }

    [BackwardDifferentiable]
    static AData toData(DataBlock datablock)
    {
        return reinterpret<AData, DataBlob>(datablock.data);
    }

    [BackwardDifferentiable]
    static DataBlock toDataBlock(AData data)
    {
        return { 0, reinterpret<DataBlob, AData>(data) };
    }

    [BackwardDifferentiable]
    static AData.Differential toDifferentialData(DataBlock.Differential datablock)
    {
        return reinterpret<AData.Differential, DataBlob>(datablock.data);
    }

    [BackwardDifferentiable]
    static DataBlock.Differential toDifferentialDataBlob(AData.Differential data)
    {
        return { 0, reinterpret<DataBlob, AData.Differential>(data) };
    }
};

struct B : IInterface
{
    typealias ThisData = BData;

    [BackwardDifferentiable]
    float compute(DataBlock data, float c)
    {
        BData bData = toData(data);
        return bData.a * bData.b * c;
    }

    [BackwardDifferentiable]
    static BData toData(DataBlock datablock)
    {
        return reinterpret<BData, DataBlob>(datablock.data);
    }

    [BackwardDifferentiable]
    static DataBlock toDataBlob(BData data)
    {
        return {1, reinterpret<DataBlob, BData>(data)};
    }

    [BackwardDifferentiable]
    static BData.Differential toDifferentialData(DataBlock.Differential datablock)
    {
        return reinterpret<BData.Differential, DataBlob>(datablock.data);
    }

    [BackwardDifferentiable]
    static DataBlock.Differential toDifferentialDataBlob(BData.Differential data)
    {
        return {1, reinterpret<DataBlob, BData.Differential>(data) };
    }
};

[BackwardDifferentiable]
float compute(IInterface i, DataBlock data, float c)
{
    return i.compute(data, c);
}

// TEST_INPUT: type_conformance A:IInterface = 0
// TEST_INPUT: type_conformance B:IInterface = 1

[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        AData a = { 1.f, 2.f };
        DifferentialPair<AData> aPair = DifferentialPair<AData>(a, AData.Differential.dzero());
        DifferentialPair<float> cPair = DifferentialPair<float>(1.f, 0.f);

        __bwd_diff(compute)(0, aPair, cPair, float(1.f));
    }
}