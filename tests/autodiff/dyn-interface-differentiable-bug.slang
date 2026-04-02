//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -compute -shaderobj -output-using-type -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

public dyn interface IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v); 
};

public struct Foo1 : IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v) 
    {
        float sum = 0;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            sum += v[i];
        return sum;
    }
};


public struct Foo2 : IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v) 
    {
        float sum = 0;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            sum += 2 * v[i];
        return sum;
    }
};

[Differentiable]
float doThing(IFoo foo, vector<float, 4> v)
{
    return foo.bar(v);
}

[shader("compute"), numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo foo;

    if (dispatchThreadID.x == 0)
        foo = Foo1();
    else
        foo = Foo2();

    DifferentialPair<vector<float, 4>> dpa = 
        diffPair(
            vector<float, 4>(1, 2, 3, 4), 
            vector<float, 4>(0, 0, 0, 0));

    bwd_diff(doThing)(foo, dpa, 1.0f);

    outputBuffer[0] = dpa.d[0]; // CHECK: 1.0
    outputBuffer[1] = dpa.d[1]; // CHECK: 1.0
}