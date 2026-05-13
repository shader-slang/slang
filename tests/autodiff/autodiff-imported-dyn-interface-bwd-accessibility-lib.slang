//TEST_IGNORE_FILE:

module "autodiff-imported-dyn-interface-bwd-accessibility-lib";

public RWStructuredBuffer<float> outputBuffer;

public dyn interface IFoo
{
    [Differentiable]
    public void add(float value);
}

[Differentiable]
public void add(IFoo foo, float value)
{
    foo.add(value);
}

public struct Foo : IFoo
{
    [Differentiable]
    public void add(float value)
    {
        outputBuffer[0] = detach(value + 10.0);
    }
}

public struct Bar : IFoo
{
    [ForwardDerivative(fwd_add), BackwardDerivative(bwd_add)]
    public void add(float value)
    {
        outputBuffer[0] = detach(value + 20.0);
    }

    public void fwd_add(DifferentialPair<float> value)
    {
    }

    public void bwd_add(inout DifferentialPair<float> value)
    {
    }
}
