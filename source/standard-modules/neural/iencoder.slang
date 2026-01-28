implementing neural;
#include "common-def.slang"

public interface IStaticEncoder<T,
    InArray,
    OutArray
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InArray : IArrayAccessor<T>
    where OutArray : IArrayAccessor<T>
{
    public associatedtype HyperParameters;

    [Differentiable]
    public static OutArray encode(in InArray input);
}

public interface ITrainableEncoder<T,
    InArray,
    OutArray
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InArray : IArrayAccessor<T>
    where InArray : IDifferentiable
    where OutArray : IArrayAccessor<T>
    where OutArray : IDifferentiable
{
    public associatedtype HyperParameters;

    [Differentiable]
    public static OutArray encode<Storage>(HyperParameters params, in InArray input, Storage parameters, no_diff in Storage.Address parametersAddress)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address;

    [Differentiable]
    public static OutArray encode<Address>(HyperParameters params, in InArray input, Address parametersAddress)
        where Address : IPointerLikeAddress<T>;
}
