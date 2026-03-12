implementing neural;

/**
Layer interface (compile-time, GPU-friendly).
This interface is intended to be used as a *generic constraint* (no existential storage),
so it does not imply dynamic dispatch.

@category neural
*/
public interface ILayer<T, InputVector, OutputVector, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    /// Forward evaluation: y = f(x).
    /// @param input Input vector.
    /// @param parameterAddr Base address of this layer's contiguous parameter block.
    ///        Weights start at offset 0; bias (if any) follows immediately.
    /// @return Output vector.
    [Differentiable]
    public OutputVector eval<A>(InputVector input, A parameterAddr)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T.Differential>;
}
