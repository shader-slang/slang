implementing neural;

/**
Layer interface (compile-time, GPU-friendly).
This interface is intended to be used as a *generic constraint* (no existential storage),
so it does not imply dynamic dispatch.

@category neural
*/
public interface ILayer<T, InputVector, OutputVector, Storage, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Storage : IStorage<T>
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    /// Forward evaluation: y = f(x).
    /// Storage is passed as parameter to enable autodiff gradient routing.
    /// Method-level generic `S` is required for `linearTransform` constraints.
    /// @param storage Weight/bias storage (primal for forward, gradient for backward via autodiff).
    /// @param input Input vector.
    /// @return Output vector.
    [Differentiable]
    public OutputVector eval<S>(S storage, InputVector input)
        where S : IStorage<T>
        where S.Differential : IStorage<T>
        where S.Address == S.Differential.Address
        where S.Address == Storage.Address;
}
