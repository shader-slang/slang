//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Regression test for synthesized subscript witnesses with `no_diff` value types.
// The concrete implementations use `uint` indices so satisfying the `int`-indexed
// interface requirements must go through subscript witness synthesis.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

interface IValue
{
    property no_diff float value { get; }
}

struct ValueImpl : IValue
{
    no_diff float value;
}

interface IIndexable
{
    __subscript(int index) -> no_diff float { get; }
}

struct IndexableImpl : IIndexable
{
    float data[2];

    __subscript(uint index) -> no_diff float
    {
        get { return data[int(index)]; }
    }
}

interface IIndexableRW
{
    __subscript(int index) -> no_diff float { get; set; }
}

struct IndexableImplRW : IIndexableRW
{
    float data[2];

    __subscript(uint index) -> no_diff float
    {
        get { return data[int(index)]; }
        set { data[int(index)] = newValue; }
    }
}

[ForwardDifferentiable]
float genericSubscriptTest<T : IIndexable>(T obj, float x)
{
    return obj[0] * x;
}

[ForwardDifferentiable]
float genericSubscriptSetTest<T : IIndexableRW, U : IValue>(T obj, U src, float y)
{
    obj[0] = src.value;
    return obj[0] * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IndexableImpl indexable;
    indexable.data[0] = 3.0;
    indexable.data[1] = 4.0;

    let genericSubscript = __fwd_diff(genericSubscriptTest)(indexable, dpfloat(2.0, 1.0));
    outputBuffer[0] = genericSubscript.p;
    outputBuffer[1] = genericSubscript.d;

    IndexableImplRW indexable2;
    indexable2.data[0] = 1.0;
    indexable2.data[1] = 0.0;

    ValueImpl src;
    src.value = 3.0;

    let genericSubscriptSet = __fwd_diff(genericSubscriptSetTest)(indexable2, src, dpfloat(2.0, 1.0));
    outputBuffer[2] = genericSubscriptSet.p;
    outputBuffer[3] = genericSubscriptSet.d;

    // CHECK: 6.0
    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 3.0
}
