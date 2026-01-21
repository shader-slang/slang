implementing neural;

__include iactivation;

/// Empty struct for activations that don't need parameters.
public struct NoParam
{
    public __init() {}
}

/**
Identity activation: returns input unchanged.
*/
public struct IdentityActivation<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
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
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output.set(i, max(input.get(i), T(0)));
        return output;
    }
}

/**
LeakyReLU activation: x < 0 ? alpha*x : x
@param alpha Leak coefficient (typically 0.01).
*/
public struct LeakyReLU<T> : IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ParamType = T;

    [Differentiable]
    public static Vector eval<Vector>(T alpha, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input.get(i);
            output.set(i, (x < T(0)) ? alpha * x : x);
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
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
        {
            let x = input.get(i);
            output.set(i, T(1) / (T(1) + exp(-x)));
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
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output.set(i, tanh(input.get(i)));
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
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output.set(i, exp(input.get(i)));
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
    typealias ParamType = NoParam;

    [Differentiable]
    public static Vector eval<Vector>(NoParam param, Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output.set(i, sin(input.get(i)));
        return output;
    }
}
