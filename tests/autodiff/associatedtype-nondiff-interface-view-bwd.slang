// Test that a method called through a non-differentiable associated-type/interface
// view does not propagate derivatives into `this` in reverse mode, even when the
// concrete type itself is differentiable.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IParams
{
    // Since IParams does not inherit from IDifferentiable, `this` should be
    // treated as `no_diff` when `getVal` is called through this view.
    [BackwardDifferentiable]
    float getVal(float y);
}

struct DiffParams : IDifferentiable, IParams
{
    float x;

    [BackwardDifferentiable]
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
        params.x = x;
        return params;
    }
}

[BackwardDifferentiable]
float fAssoc<T : IFoo>(T::Params params, float y)
{
    return params.getVal(y);
}

[BackwardDifferentiable]
float fCast(DiffParams params, float y)
{
    IParams p = params;
    return p.getVal(y);
}

[BackwardDifferentiable]
float fConcrete(DiffParams params, float y)
{
    return params.getVal(y);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let params = Foo::decode<4>(2.0f);

    {
        var dpy = diffPair(3.0f);
        // `T::Params` is treated as non-differentiable in this context, so
        // reverse mode only accepts/propagates derivative for `y`.
        bwd_diff(fAssoc<Foo>)(params, dpy, 1.0f);

        outputBuffer[0] = fAssoc<Foo>(params, 3.0f); // CHECK: 13.0
        outputBuffer[1] = dpy.d; // CHECK: 6.0
    }

    {
        var dpParams = diffPair(params);
        var dpy = diffPair(3.0f);
        bwd_diff(fCast)(dpParams, dpy, 1.0f);

        outputBuffer[2] = fCast(params, 3.0f); // CHECK: 13.0
        outputBuffer[3] = dpy.d; // CHECK: 6.0
        outputBuffer[4] = dpParams.d.x; // CHECK: 0.0
    }

    {
        var dpParams = diffPair(params);
        var dpy = diffPair(3.0f);
        bwd_diff(fConcrete)(dpParams, dpy, 1.0f);

        outputBuffer[5] = fConcrete(params, 3.0f); // CHECK: 13.0
        outputBuffer[6] = dpy.d; // CHECK: 6.0
        outputBuffer[7] = dpParams.d.x; // CHECK: 4.0
    }
}
