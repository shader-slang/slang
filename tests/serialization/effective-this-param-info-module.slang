//TEST_IGNORE_FILE:

module "effective-this-param-info-module";

public struct Receiver
{
    public int value;

    [constref]
    public int read()
    {
        return value;
    }

    [__ref]
    public void increment()
    {
        value++;
    }
}

public struct DifferentiableReceiver : IDifferentiable
{
    public typealias Differential = DifferentiableReceiver;
    public float value;

    [Differentiable]
    public static Differential dzero()
    {
        return {0.0};
    }

    [Differentiable]
    public static Differential dadd(Differential left, Differential right)
    {
        return {left.value + right.value};
    }

    [NoDiffThis]
    [ForwardDifferentiable]
    public float scale(float factor)
    {
        return value * factor;
    }
}
