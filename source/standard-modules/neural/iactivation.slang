implementing neural;

/**
Activation function interface for neural network operations.
Defines a differentiable mapping from an input vector to an output vector of the same shape.

@param T Scalar element type (float/half/double).
@category neural
*/
public interface IActivation<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// Parameter type for this activation (e.g., float for LeakyReLU alpha, void for ReLU).
    associatedtype ParamType;

    /// Apply activation function element-wise.
    /// @param param Activation-specific parameters (ignored if ParamType is void).
    /// @param input Input vector.
    /// @return Output vector with activation applied.
    [Differentiable]
    public static Vector eval<Vector>(ParamType param, Vector input)
        where Vector : IVector<T>;
}
