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
    public OutArray encode(in InArray input);
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
    public OutArray encode<Address>(in InArray input, Address parametersAddress)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T.Differential>;
}
