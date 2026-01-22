implementing neural;

/**
Layer interface (compile-time, GPU-friendly).
This interface is intended to be used as a *generic constraint* (no existential storage),
so it does not imply dynamic dispatch.

@category neural
*/
public interface ILayer<T, InputVector, OutputVector, Storage, Activation>
    where T : __BuiltinFloatingPointType
    where T == T.Differential
    where Storage : IStorage<T>
    where Storage.Differential : IStorage<T.Differential>
    where Storage.Address == Storage.Differential.Address
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    /// Evaluate layer: y = f(x).
    /// @param activationParam Activation-specific parameters.
    /// @param input Input vector.
    [NoDiffThis, Differentiable]
    public OutputVector eval(Activation.ParamType activationParam, InputVector input);

    /// Number of scalar parameters consumed by the layer (for contiguous packing conventions).
    public static const int ParameterCount;

    /// Returns the address immediately after this layer's parameters (for chaining layers).
    public static Storage.Address nextAddress(Storage.Address baseAddress);
}
