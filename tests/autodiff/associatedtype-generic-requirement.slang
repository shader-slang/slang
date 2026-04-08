// Test autodiff-marked generic interface requirement with associated-type return.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IParams : IDifferentiable
{
    [Differentiable]
    float get();
}

struct DiffParams : IParams
{
    float value;

    [Differentiable]
    float get()
    {
        return value;
    }
}

interface IFoo
{
    associatedtype Params : IParams;

    [BackwardDifferentiable]
    static Params decode<let N : uint>(no_diff float x);
}

struct Foo : IFoo
{
    typealias Params = DiffParams;

    [BackwardDifferentiable]
    static Params decode<let N : uint>(no_diff float x)
    {
        Params params;
        params.value = x + float(N);
        return params;
    }
}

[BackwardDifferentiable]
float apply<T : IFoo>(float y)
{
    return y * T::decode<4>(1.0f).get();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = apply<Foo>(3.0f); // CHECK: 15.0

    var dy = diffPair(3.0f);
    bwd_diff(apply<Foo>)(dy, 1.0f);
    outputBuffer[1] = dy.d; // CHECK: 5.0
}
