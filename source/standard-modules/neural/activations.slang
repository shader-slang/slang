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

    [ForceInline]
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
    [HasTrivialForwardDerivative]
    [BackwardDerivative(evalBackward)]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>
    {
        Vector output = Vector();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            output[i] = max(input[i], T(0));
        return output;
    }

    [NoDiffThis]
    public void evalBackward<Vector>(
        inout DifferentialPair<Vector> input,
        Vector.Differential outputGradient)
        where Vector : IVector<T>
        where Vector.Differential : IVector<T>
    {
        Vector.Differential inputGradient = Vector.Differential();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; i++)
            inputGradient[i] = input.p[i] > T(0) ? outputGradient[i] : T(0);
        input = DifferentialPair<Vector>(input.p, inputGradient);
    }
}

/**
FP16 ReLU activation that checkpoints one sign bit per vector element.

This activation is useful when reverse-mode differentiation would otherwise retain the complete
pre-activation vector. Its forward and backward operations are identical to `ReLU<half>`; only the
representation of the reverse-mode context differs.
*/
public struct CompactReLUContext<Vector : IVector<half>>
{
    public static const int MaskWordCount = (Vector.Size + 31) / 32;
    public uint positiveMask[MaskWordCount];

    public void operator()(
        out Vector.Differential inputGradient,
        Vector.Differential outputGradient)
    {
        inputGradient = Vector.Differential();
        [ForceUnroll]
        for (int i = 0; i < Vector.Size; ++i)
        {
            bool positive =
                (positiveMask[uint(i) >> 5] & (1u << (uint(i) & 31))) != 0;
            inputGradient[i] = positive ? outputGradient[i] : 0.0h;
        }
    }
}

/// Evaluates an FP16 ReLU. Its `__apply` extension supplies a compact reverse-mode context.
public Vector compactReLU<Vector : IVector<half>>(Vector input)
{
    Vector output = Vector();
    [ForceUnroll]
    for (int i = 0; i < Vector.Size; ++i)
        output[i] = max(input[i], 0.0h);
    return output;
}

public __func_extension<Vector : IVector<half>>
fwd_diff(compactReLU<Vector>)(DifferentialPair<Vector> input)
    -> DifferentialPair<Vector>
{
    Vector primal = Vector();
    Vector.Differential differential = Vector.Differential();
    [ForceUnroll]
    for (int i = 0; i < Vector.Size; ++i)
    {
        primal[i] = max(input.p[i], 0.0h);
        differential[i] = input.p[i] > 0.0h ? input.d[i] : 0.0h;
    }
    return diffPair(primal, differential);
}

public __func_extension<Vector : IVector<half>>
__apply(compactReLU<Vector>)(Vector input) -> Tuple<Vector, CompactReLUContext<Vector>>
{
    Vector output = Vector();
    CompactReLUContext<Vector> context = {};
    [ForceUnroll]
    for (int i = 0; i < Vector.Size; ++i)
    {
        half value = input[i];
        bool positive = value > 0.0h;
        output[i] = max(value, 0.0h);
        if (positive)
            context.positiveMask[uint(i) >> 5] |= 1u << (uint(i) & 31);
    }
    return makeTuple(output, context);
}

/**
FP16 ReLU activation with a bit-packed reverse-mode checkpoint.
*/
public struct CompactReLU : IActivation<half>
{
    public __init() {}

    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<half>
    {
        return compactReLU<Vector>(input);
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
            // Numerically stable: exp() argument is always <= 0
            let x = input[i];
            let ex = exp(x >= T(0) ? -x : x);
            output[i] = x >= T(0) ? T(1) / (T(1) + ex) : ex / (T(1) + ex);
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
            // x * sigmoid(x), numerically stable: exp() argument is always <= 0
            let x = input[i];
            let ex = exp(x >= T(0) ? -x : x);
            output[i] = x >= T(0) ? x / (T(1) + ex) : x * ex / (T(1) + ex);
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
            // x * sigmoid(1.702 * x), numerically stable: exp() argument is always <= 0
            let x = input[i];
            let sx = T(1.702) * x;
            let ex = exp(sx >= T(0) ? -sx : sx);
            output[i] = sx >= T(0) ? x / (T(1) + ex) : x * ex / (T(1) + ex);
        }
        return output;
    }
}
