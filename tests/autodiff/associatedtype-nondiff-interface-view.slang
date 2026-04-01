// Test that an associated type seen through a non-differentiable interface
// uses the non-propagating method view, even when the concrete type itself
// is differentiable.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

interface IParams
{
    // Since IParams does not inherit from IDifferentiable, the `this`
    // parameter should be treated as `no_diff` when called through this view.
    [ForwardDifferentiable]
    float getVal(float y);
}

struct DiffParams : IDifferentiable, IParams
{
    float x;

    [ForwardDifferentiable]
    float getVal(float y)
    {
        return x * x + y * y;
    }
}

interface IFoo
{
    associatedtype Params : IParams;

    static Params decode<let N : uint>(no_diff float x);
}

struct Foo : IFoo
{
    typealias Params = DiffParams;

    static Params decode<let N : uint>(no_diff float x)
    {
        Params params;
        params.x = x + float(N);
        return params;
    }
}

[ForwardDifferentiable]
float fGeneric<T : IFoo>(float y)
{
    return T::decode<4>(2.0f).getVal(y);
}

[ForwardDifferentiable]
float fInterface(IParams params, float y)
{
    return params.getVal(y);
}

[ForwardDifferentiable]
float fConcrete(DiffParams params, float y)
{
    return params.getVal(y);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let params = Foo::decode<4>(2.0f);

    let rsGeneric = __fwd_diff(fGeneric<Foo>)(dpfloat(3.0f, 1.0f));
    outputBuffer[0] = rsGeneric.p; // CHECK: 45.0
    outputBuffer[1] = rsGeneric.d; // CHECK: 6.0

    let rsInterface = __fwd_diff(fInterface)(params, dpfloat(3.0f, 1.0f));
    outputBuffer[2] = rsInterface.p; // CHECK: 45.0
    outputBuffer[3] = rsInterface.d; // CHECK: 6.0

    DiffParams.Differential dParams;
    dParams.x = 1.0f;
    let rsConcrete = __fwd_diff(fConcrete)(DifferentialPair<DiffParams>(params, dParams), dpfloat(3.0f, 1.0f));
    outputBuffer[4] = rsConcrete.p; // CHECK: 45.0
    outputBuffer[5] = rsConcrete.d; // CHECK: 18.0
}
