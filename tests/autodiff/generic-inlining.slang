//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;


public interface IFoo
{
    float read(uint id);
    void write(uint id, float value);
}

public extension<Foo : IFoo> Foo
{
    [ForceInline]
    [Differentiable]
    [BackwardDerivative(_load_bwd_indices)]
    public float load(int i0)
    {
        return this.read(i0);
    }

    public void _load_bwd_indices(int i0, float dOut)
    {
        write(i0, dOut);
    }
}

struct FooImpl : IFoo
{
    float read(uint id) { return outputBuffer[0]; }
    void write(uint id, float value) { outputBuffer[0] = value; }
}

[Differentiable]
float test(IFoo foo)
{
    return (foo.load(0) + 2) * (foo.load(0) + 2);
}

[numthreads(1,1,1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    FooImpl foo;
    bwd_diff(test)(foo, 1.0);
    // CHECK: 2
}
