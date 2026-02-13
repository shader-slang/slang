implementing neural;

/**
Layer interface (compile-time, GPU-friendly).
This interface is intended to be used as a *generic constraint* (no existential storage),
so it does not imply dynamic dispatch.

@category neural
*/
public interface ILayer<T, InputVector, OutputVector, Address, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Address : IPointerLikeAddress<T>
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    /// Forward evaluation: y = f(x).
    /// Address is passed as parameter to enable autodiff gradient routing.
    /// @param weightAddress Weight address (pointer-like).
    /// @param biasAddress Bias address (pointer-like).
    /// @param input Input vector.
    /// @return Output vector.
    [Differentiable]
    public OutputVector eval<A>(A weightAddress, A biasAddress, InputVector input)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T.Differential>;
}
