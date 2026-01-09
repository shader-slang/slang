implementing neural;

__include iactivation;
__include ilayer;
__include "buffer-storage";

/**
Layer parameters packed as a contiguous block:

- weights: Out * In scalars, row-major by output row
  W[row * In + col]
- bias (optional): Out scalars

`FFLayer` is generic over `Storage : IStorage<T>`. Address arithmetic is performed
through `Storage.getOffset()` so different storage backends can define their own
addressing scheme.

Note: Storage cannot be stored as a member field because `linearTransform` requires
method-level generic constraints that can only be verified at call-site. The storage
must be passed as a parameter of type `S` to satisfy these constraints.
*/
public struct FFLayer<
    T,
    InputVector,
    OutputVector,
    Storage,
    Activation,
    let HasBias : bool = true
>
    : ILayer<T, InputVector, OutputVector, Storage, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Storage : IStorage<T>
    where Activation : IActivation<T>
{
    public static const int ParameterCount =
        OutputVector.Size * InputVector.Size + (HasBias ? OutputVector.Size : 0);

    internal Storage.Address weightAddress;
    internal Storage.Address biasAddress;

    /// Constructor for layers without bias.
    public __init(Storage.Address weightAddr)
    {
        weightAddress = weightAddr;
    }

    /// Constructor for layers with bias (separate addresses).
    /// Weight and bias can be in different storage locations.
    public __init(Storage.Address weightAddr, Storage.Address biasAddr)
    {
        weightAddress = weightAddr;
        biasAddress = biasAddr;
    }

    public static Storage.Address nextAddress(Storage.Address baseAddress)
    {
        return Storage.getOffset(baseAddress, ParameterCount);
    }

    /// Evaluate layer: y = Activation(W*x + b).
    /// Method-level generic `S` is required for `linearTransform` constraints.
    /// @param storage Weight/bias storage (passed as `S` to satisfy constraints).
    /// @param activationParam Activation-specific parameters (e.g., alpha for LeakyReLU).
    /// @param input Input vector.
    [NoDiffThis, Differentiable]
    public OutputVector eval<S>(S storage, Activation.ParamType activationParam, InputVector input)
        where S : IStorage<T>
        where S.Differential : IStorage<T.Differential>
        where S.Address == S.Differential.Address
        where S.Address == Storage.Address
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<S, OutputVector>(
                storage, storage, S.Address(weightAddress), S.Address(biasAddress));
        }
        else
        {
            y = input.linearTransform<S, OutputVector>(
                storage, S.Address(weightAddress));
        }
        return Activation.eval<OutputVector>(activationParam, y);
    }

    /// Evaluate layer using Address-based API (bindless/pointer-like storage).
    /// Caller provides addresses directly; does not use stored addresses.
    /// @param weightAddr Weight address (pointer-like).
    /// @param biasAddr Bias address (pointer-like). Ignored if HasBias=false.
    /// @param activationParam Activation-specific parameters.
    /// @param input Input vector.
    [Differentiable]
    public static OutputVector eval<A>(A weightAddr, A biasAddr, Activation.ParamType activationParam, InputVector input)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T.Differential>
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<A, OutputVector>(weightAddr, biasAddr);
        }
        else
        {
            y = input.linearTransform<A, OutputVector>(weightAddr);
        }
        return Activation.eval<OutputVector>(activationParam, y);
    }
}
