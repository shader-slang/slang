implementing neural;
#include "common-def.slang"

public interface IEncoder<T,
    InVector,
    OutVector
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InVector : IVector<T>
    where OutVector : IVector<T>
{
    associatedtype HyperParameters;
}

public interface IStaticEncoder<T,
    InVector,
    OutVector
    > : IEncoder<T, InVector, OutVector>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InVector : IVector<T>
    where OutVector : IVector<T>
{
    [Differentiable]
    OutVector encode(in InVector input);
}

public interface ITrainableEncoder<T,
    InVector,
    OutVector
    > : IEncoder<T, InVector, OutVector>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InVector : IVector<T>
    where OutVector : IVector<T>
{
    [Differentiable]
    public OutVector encode<Storage>(in InVector input, Storage parameters, no_diff in Storage.Address parametersAddress)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address;

    [Differentiable]
    public OutVector encode<Address>( in InVector input, Address parametersAddress)
        where Address : IPointerLikeAddress<T>;
}
