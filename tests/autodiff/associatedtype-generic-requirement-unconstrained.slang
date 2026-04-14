// Test that an autodiff-marked static generic requirement returning an
// unconstrained associated type now conforms correctly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct DiffParams : IDifferentiable
{
    float value;
}

interface IFoo
{
    associatedtype Params;

    [BackwardDifferentiable]
    static Params decode<let N : uint>();
}

struct Foo : IFoo
{
    typealias Params = DiffParams;

    [BackwardDifferentiable]
    static Params decode<let N : uint>()
    {
        Params params;
        params.value = float(N) + 1.0f;
        return params;
    }
}

DiffParams makeFooParams()
{
    return Foo::decode<4>();
}

[BackwardDifferentiable]
float apply<T : IFoo>(float y)
{
    let params = T::decode<4>();
    return y * y;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = makeFooParams().value; // CHECK: 5.0
    outputBuffer[1] = apply<Foo>(3.0f); // CHECK: 9.0

    var dy = diffPair(3.0f);
    bwd_diff(apply<Foo>)(dy, 1.0f);
    outputBuffer[2] = dy.d; // CHECK: 6.0
}
