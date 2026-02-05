implementing neural;

/**
Activation function interface for neural network operations.
Defines a differentiable mapping from an input vector to an output vector of the same shape.

Activations that require parameters (e.g., LeakyReLU's alpha) store them as member fields.
Parameterless activations (e.g., ReLU, Sigmoid) can be used with the default constructor.

@param T Scalar element type (float/half/double).
@category neural
*/
public interface IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// Default constructor. Required so that activations are explicitly default-constructed
    /// rather than zero-initialized when used with `Activation()` in layer constructors.
    __init();

    /// Apply activation function element-wise.
    /// @param input Input vector.
    /// @return Output vector with activation applied.
    [NoDiffThis]
    [Differentiable]
    public Vector eval<Vector>(Vector input)
        where Vector : IVector<T>;
}
