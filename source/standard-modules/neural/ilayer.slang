implementing neural;

/**
Layer interface (compile-time, GPU-friendly).
This interface is intended to be used as a *generic constraint* (no existential storage),
so it does not imply dynamic dispatch.

Note: The eval method uses a method-level generic `S` because constraints like
`Storage.Differential : IStorage<T.Differential>` cannot be verified at struct-level
when `Storage` is abstract. These constraints can only be checked at call-site when
`S` is concrete.

@category neural
*/
public interface ILayer<T, InputVector, OutputVector, Storage, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Storage : IStorage<T>
    where Activation : IActivation<T>
{
    /// Evaluate layer: y = f(x).
    /// Method-level generic `S` is required for `linearTransform` constraints.
    /// @param storage Weight/bias storage.
    /// @param activationParam Activation-specific parameters.
    /// @param input Input vector.
    [NoDiffThis, Differentiable]
    public OutputVector eval<S>(S storage, Activation.ParamType activationParam, InputVector input)
        where S : IStorage<T>
        where S.Differential : IStorage<T.Differential>
        where S.Address == S.Differential.Address
        where S.Address == Storage.Address;

    /// Number of scalar parameters consumed by the layer (for contiguous packing conventions).
    public static const int ParameterCount;

    /// Returns the address immediately after this layer's parameters (for chaining layers).
    public static Storage.Address nextAddress(Storage.Address baseAddress);
}
