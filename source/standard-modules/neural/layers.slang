implementing neural;

__include iactivation;
__include ilayer;
__include "buffer-storage";

/**
A fully-connected (feed-forward) neural network layer that computes `y = Activation(W*x + b)`.

`FFLayer` represents a single linear transformation followed by an activation function,
suitable for building multi-layer perceptrons (MLPs) and similar architectures.

## Usage

1. **Construction:** Create a layer with weight/bias addresses pointing into your storage:
   ```
   let layer = FFLayer<float, Vec4, Vec2, Storage, LinearLayout, ReLU<float>>(weightAddr, biasAddr);
   ```

2. **Forward pass:** Call `eval()` with storage and input:
   ```
   let output = layer.eval<Storage>(storage, input);
   ```

3. **Training (backward pass):** Use autodiff with `DifferentialPtrPair`:
   ```
   var storagePair = DifferentialPtrPair<Storage>(storage, gradStorage);
   bwd_diff(computeOutput)(storagePair, inputPair, layer, dOutput);
   ```

4. **With custom activation parameters** (e.g., LeakyReLU with custom alpha):
   ```
   let leakyRelu = LeakyReLU<float>(0.2);  // alpha = 0.2
   let layer = FFLayer<float, Vec4, Vec2, Storage, LinearLayout, LeakyReLU<float>>(weightAddr, biasAddr, leakyRelu);
   ```

## Parameter Layout

Parameters are packed as a contiguous block in storage:

- **weights:** `Out * In` scalars, row-major by output row: `W[row * In + col]`
- **bias (optional):** `Out` scalars immediately following weights

`FFLayer` is generic over `Storage : IStorage<T>`. Address arithmetic uses
`Storage.getOffset()` so different storage backends can define their own addressing scheme.
*/
public struct FFLayer<
    T,
    InputVector,
    OutputVector,
    Storage,
    Layout,
    Activation,
    let HasBias : bool = true
>
    : ILayer<T, InputVector, OutputVector, Storage, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Storage : IStorage<T>
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    public static const int ParameterCount =
        OutputVector.Size * InputVector.Size + (HasBias ? OutputVector.Size : 0);

    /// Addresses are stored as members (integers, not differentiable).
    internal Storage.Address weightAddress;
    internal Storage.Address biasAddress;

    /// Activation function instance (stores any activation-specific parameters).
    internal Activation activation;

    /// Constructor for layers without bias.
    /// @param weightAddr Address of weights in storage.
    /// @param act Activation function instance (defaults to default-constructed Activation).
    public __init(Storage.Address weightAddr, Activation act = Activation())
    {
        weightAddress = weightAddr;
        activation = act;
    }

    /// Constructor for layers with bias.
    /// @param weightAddr Address of weights in storage.
    /// @param biasAddr Address of bias in storage.
    /// @param act Activation function instance (defaults to default-constructed Activation).
    public __init(Storage.Address weightAddr, Storage.Address biasAddr, Activation act = Activation())
    {
        weightAddress = weightAddr;
        biasAddress = biasAddr;
        activation = act;
    }

    public static Storage.Address nextAddress(Storage.Address baseAddress)
    {
        return Storage.getOffset(baseAddress, ParameterCount);
    }

    /// Forward evaluation: y = Activation(W*x + b).
    /// Storage is passed as parameter to enable autodiff gradient routing.
    /// @param storage Weight/bias storage (primal for forward, gradient for backward via autodiff).
    /// @param input Input vector.
    /// @return Output vector after linear transform and activation.
    [Differentiable]
    public OutputVector eval<S>(S storage, InputVector input)
        where S : IStorage<T>
        where S.Differential : IStorage<T.Differential>
        where S.Address == S.Differential.Address
        where S.Address == Storage.Address
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<S, Layout, OutputVector>(
                storage, storage, weightAddress, biasAddress);
        }
        else
        {
            y = input.linearTransform<S, Layout, OutputVector>(
                storage, weightAddress);
        }
        return activation.eval<OutputVector>(y);
    }

    /// Forward evaluation with weights and bias coming from different storages.
    /// This is useful when weights and bias are stored in separate buffers (or otherwise
    /// distinct storage instances).
    [Differentiable]
    public OutputVector eval<S>(
        S weightStorage,
        S biasStorage,
        InputVector input)
            where S : IStorage<T>
            where S.Differential : IStorage<T.Differential>
            where S.Address == S.Differential.Address
            where S.Address == Storage.Address
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<S, Layout, OutputVector>(
                weightStorage, biasStorage, weightAddress, biasAddress);
        }
        else
        {
            // Bias is disabled; ignore biasStorage.
            y = input.linearTransform<S, Layout, OutputVector>(
                weightStorage, weightAddress);
        }
        return activation.eval<OutputVector>(y);
    }

    /// Evaluate layer using Address-based API (bindless/pointer-like storage).
    /// Caller provides addresses directly; does not use stored addresses.
    /// @param weightAddr Weight address (pointer-like).
    /// @param biasAddr Bias address (pointer-like). Ignored if HasBias=false.
    /// @param act Activation function instance.
    /// @param input Input vector.
    [Differentiable]
    public static OutputVector eval<A>(A weightAddr, A biasAddr, Activation act, InputVector input)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T.Differential>
    {
        OutputVector y;
        if(HasBias)
        {
            y = input.linearTransform<A, Layout, OutputVector>(weightAddr, biasAddr);
        }
        else
        {
            y = input.linearTransform<A, Layout, OutputVector>(weightAddr);
        }
        return act.eval<OutputVector>(y);
    }
}
