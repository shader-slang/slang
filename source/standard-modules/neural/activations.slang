implementing neural;

__include iactivation;

/**
Identity activation: returns input unchanged.
*/
public struct IdentityActivation<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        return input;
    }
}

/**
ReLU activation: max(x, 0).
*/
public struct ReLU<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output[i] = max(input[i], T(0));
        return output;
    }
}

/**
LeakyReLU activation: x < 0 ? alpha*x : x

Construct with the leak coefficient alpha (typically 0.01).
*/
public struct LeakyReLU<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// Leak coefficient for negative inputs.
    public T alpha;

    /// Constructor with optional alpha value (defaults to 0.01).
    public __init(T alpha = T(0.01))
    {
        this.alpha = alpha;
    }

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input[i];
            output[i] = (x < T(0)) ? alpha * x : x;
        }
        return output;
    }
}

/**
Sigmoid activation: 1 / (1 + exp(-x))
*/
public struct Sigmoid<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input[i];
            output[i] = T(1) / (T(1) + exp(-x));
        }
        return output;
    }
}

/**
Tanh activation.
*/
public struct TanhActivation<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output[i] = tanh(input[i]);
        return output;
    }
}

/**
Exp activation: exp(x)
*/
public struct ExpActivation<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output[i] = exp(input[i]);
        return output;
    }
}

/**
Sine activation: sin(x)
*/
public struct SineActivation<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output[i] = sin(input[i]);
        return output;
    }
}

/**
SiLU (Sigmoid Linear Unit) activation, also known as Swish: x * sigmoid(x)
*/
public struct SiLU<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input[i];
            output[i] = x / (T(1) + exp(-x));  // x * sigmoid(x)
        }
        return output;
    }
}

/**
QuickGELU activation: x * sigmoid(1.702 * x)

A fast approximation of GELU (Gaussian Error Linear Unit).
*/
public struct QuickGELU<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input[i];
            output[i] = x / (T(1) + exp(T(-1.702) * x));  // x * sigmoid(1.702 * x)
        }
        return output;
    }
}
