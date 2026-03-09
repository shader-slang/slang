// Nested Tuple<Tuple<T1, T2>, T3> as a field in interface implementations.
// Tests that AnyValue packing handles nested struct-lowered tuples correctly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ICompute
{
    float first();
    float second();
    float third();
}

struct NestedPair : ICompute
{
    Tuple<Tuple<int, float>, float> data;

    __init(int a, float b, float c)
    {
        data = makeTuple(makeTuple(a, b), c);
    }

    float first() { return float(data._0._0); }
    float second() { return data._0._1; }
    float third() { return data._1; }
}

struct FlatTriple : ICompute
{
    Tuple<float, float, float> data;

    __init(float a, float b, float c)
    {
        data = makeTuple(a, b, c);
    }

    float first() { return data._0; }
    float second() { return data._1; }
    float third() { return data._2; }
}

ICompute makeCompute(int id)
{
    if (id == 0)
        return NestedPair(10, 20.0, 30.0);
    else
        return FlatTriple(1.5, 2.5, 3.5);
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // NestedPair: (10, 20.0), 30.0
    var a = makeCompute(id);
    outputBuffer[0] = a.first();
    // CHECK: 10.0
    outputBuffer[1] = a.second();
    // CHECK: 20.0
    outputBuffer[2] = a.third();
    // CHECK: 30.0

    // FlatTriple: 1.5, 2.5, 3.5
    var b = makeCompute(id + 1);
    outputBuffer[3] = b.first();
    // CHECK: 1.5
    outputBuffer[4] = b.second();
    // CHECK: 2.5
    outputBuffer[5] = b.third();
    // CHECK: 3.5
}
