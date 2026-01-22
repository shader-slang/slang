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
    where T == T.Differential
    where Storage : IStorage<T>
    where Storage.Differential : IStorage<T.Differential>
    where Storage.Address == Storage.Differential.Address
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    public static const int ParameterCount =
        OutputVector.Size * InputVector.Size + (HasBias ? OutputVector.Size : 0);

    internal Storage storage;
    internal Storage.Address weightAddress;
    internal Storage.Address biasAddress;

    /// Constructor for layers without bias.
    public __init(Storage s, Storage.Address weightAddr)
    {
        storage = s;
        weightAddress = weightAddr;
    }

    /// Constructor for layers with bias (separate addresses).
    /// Weight and bias can be in different storage locations.
    public __init(Storage s, Storage.Address weightAddr, Storage.Address biasAddr)
    {
        storage = s;
        weightAddress = weightAddr;
        biasAddress = biasAddr;
    }

    public static Storage.Address nextAddress(Storage.Address baseAddress)
    {
        return Storage.getOffset(baseAddress, ParameterCount);
    }

    /// Evaluate layer: y = Activation(W*x + b).
    /// @param activationParam Activation-specific parameters (e.g., alpha for LeakyReLU).
    /// @param input Input vector.
    [NoDiffThis, Differentiable]
    public OutputVector eval(Activation.ParamType activationParam, InputVector input)
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<Storage, LinearLayout, OutputVector>(
                storage, storage, weightAddress, biasAddress);
        }
        else
        {
            y = input.linearTransform<Storage, LinearLayout, OutputVector>(
                storage, weightAddress);
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
            y = input.linearTransform<A, LinearLayout, OutputVector>(weightAddr, biasAddr);
        }
        else
        {
            y = input.linearTransform<A, LinearLayout, OutputVector>(weightAddr);
        }
        return Activation.eval<OutputVector>(activationParam, y);
    }
}
