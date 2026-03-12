// Tuple<T1, T2> as a field in interface implementations. Tuples are
// lowered to structs before AnyValue marshalling, so this verifies
// the lowering + packing pipeline for concrete-typed tuple fields.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IData
{
    float sum();
    float product();
}

struct PairData : IData
{
    Tuple<float, int> pair;

    __init(float f, int i)
    {
        pair = makeTuple(f, i);
    }

    float sum() { return pair._0 + float(pair._1); }
    float product() { return pair._0 * float(pair._1); }
}

struct TripleData : IData
{
    Tuple<float, float, float> triple;

    __init(float a, float b, float c)
    {
        triple = makeTuple(a, b, c);
    }

    float sum() { return triple._0 + triple._1 + triple._2; }
    float product() { return triple._0 * triple._1 * triple._2; }
}

struct VecTupleData : IData
{
    Tuple<float3, float> data;

    __init(float3 v, float s)
    {
        data = makeTuple(v, s);
    }

    float sum() { return data._0.x + data._0.y + data._0.z + data._1; }
    float product() { return data._0.x * data._0.y * data._0.z * data._1; }
}

IData makeData(int id)
{
    switch (id)
    {
    case 0: return PairData(3.0, 4);
    case 1: return TripleData(2.0, 3.0, 5.0);
    default: return VecTupleData(float3(1.0, 2.0, 3.0), 4.0);
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // PairData: sum = 3.0 + 4.0 = 7.0, product = 3.0 * 4.0 = 12.0
    var a = makeData(id);
    outputBuffer[0] = a.sum();
    // CHECK: 7.0
    outputBuffer[1] = a.product();
    // CHECK: 12.0

    // TripleData: sum = 2+3+5 = 10.0, product = 2*3*5 = 30.0
    var b = makeData(id + 1);
    outputBuffer[2] = b.sum();
    // CHECK: 10.0
    outputBuffer[3] = b.product();
    // CHECK: 30.0

    // VecTupleData: sum = 1+2+3+4 = 10.0, product = 1*2*3*4 = 24.0
    var c = makeData(id + 2);
    outputBuffer[4] = c.sum();
    // CHECK: 10.0
    outputBuffer[5] = c.product();
    // CHECK: 24.0
}
