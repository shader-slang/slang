//TEST_IGNORE_FILE:

public struct Operations
{
    [Differentiable]
    public float square(float x) { return x * x; }

    [Differentiable]
    public T genericSquare<T : __BuiltinFloatingPointType>(T x) { return x * x; }

    [ForwardDerivative(customForward)]
    public float custom(float x) { return x * x; }

    public DifferentialPair<float> customForward(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 3.0 * x.p * x.d);
    }

    [BackwardDerivative(customBackward)]
    public float customReverse(float x) { return x * x; }

    public void customBackward(inout DifferentialPair<float> x, float derivative)
    {
        x = diffPair(x.p, 5.0 * x.p * derivative);
    }

    public float decoratedElsewhere(float x) { return x * x; }

    [ForwardDerivativeOf(decoratedElsewhere)]
    public DifferentialPair<float> forwardOf(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 7.0 * x.p * x.d);
    }

    [BackwardDerivativeOf(decoratedElsewhere)]
    public void backwardOf(inout DifferentialPair<float> x, float derivative)
    {
        x = diffPair(x.p, 11.0 * x.p * derivative);
    }

    // Derivatives attached through inverse placement record an association on the primal, and
    // their `[require]` capabilities apply where the primal is differentiated.
    public static float spirvBackwardPrimal(float x) { return x * x; }

    [require(spirv)]
    [BackwardDerivativeOf(spirvBackwardPrimal)]
    public static void spirvBackward(inout DifferentialPair<float> x, float derivative)
    {
        x = diffPair(x.p, 13.0 * x.p * derivative);
    }

    public static float spirvForwardPrimal(float x) { return x * x; }

    [require(spirv)]
    [ForwardDerivativeOf(spirvForwardPrimal)]
    public static DifferentialPair<float> spirvForward(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 13.0 * x.p * x.d);
    }

    [ForwardDerivative(internalForward)]
    public float restricted(float x) { return x * x; }

    internal DifferentialPair<float> internalForward(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 7.0 * x.p * x.d);
    }
}

public extension Operations
{
    public float ordinaryExtension(float x) { return x; }
}

public struct Samples
{
    public RWStructuredBuffer<float> values;
    public RWStructuredBuffer<float> gradients;

    [BackwardDerivative(readBackward)]
    public float read(int index) { return values[index]; }

    public void readBackward(int index, float derivative)
    {
        gradients[index] += derivative;
    }

    [Differentiable]
    public float sample(no_diff int index) { return read(index); }
}
