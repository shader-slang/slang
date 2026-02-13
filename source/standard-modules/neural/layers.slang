implementing neural;

/**
A fully-connected (feed-forward) neural network layer that computes `y = Activation(W*x + b)`.

`FFLayer` represents a single linear transformation followed by an activation function,
suitable for building multi-layer perceptrons (MLPs) and similar architectures.

## Usage

1. **Construction:** Create a layer with weight/bias offsets:
   ```
   let layer = FFLayer<float, Vec4, Vec2, Address, LinearLayout, ReLU<float>>(weightOffset, biasOffset);
   ```

2. **Forward pass:** Call `eval()` with address and input:
   ```
   let output = layer.eval<Address>(weightAddr, biasAddr, input);
   ```

3. **Training (backward pass):** Use autodiff with `DifferentialPtrPair`:
   ```
   var addrPair = DifferentialPtrPair<Address>(addr, gradAddr);
   bwd_diff(computeOutput)(addrPair, inputPair, layer, dOutput);
   ```

## Parameter Layout

Parameters are packed as a contiguous block in storage:

- **weights:** `Out * In` scalars, row-major by output row: `W[row * In + col]`
- **bias (optional):** `Out` scalars immediately following weights
*/
public struct FFLayer<
    T,
    InputVector,
    OutputVector,
    Address,
    Layout,
    Activation,
    let HasBias : bool = true
>
    : ILayer<T, InputVector, OutputVector, Address, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Address : IPointerLikeAddress<T>
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    public static const int ParameterCount =
        OutputVector.Size * InputVector.Size + (HasBias ? OutputVector.Size : 0);

    /// Weight offset (integer offset from base address).
    internal int weightOffset;
    /// Bias offset (integer offset from base address).
    internal int biasOffset;

    /// Activation function instance (stores any activation-specific parameters).
    internal Activation activation;

    /// Constructor for layers without bias.
    /// @param weightOff Offset of weights from base address.
    /// @param act Activation function instance (defaults to default-constructed Activation).
    public __init(int weightOff, Activation act = Activation())
    {
        weightOffset = weightOff;
        biasOffset = 0;
        activation = act;
    }

    /// Constructor for layers with bias.
    /// @param weightOff Offset of weights from base address.
    /// @param biasOff Offset of bias from base address.
    /// @param act Activation function instance (defaults to default-constructed Activation).
    public __init(int weightOff, int biasOff, Activation act = Activation())
    {
        weightOffset = weightOff;
        biasOffset = biasOff;
        activation = act;
    }

    public static int nextOffset(int baseOffset)
    {
        return baseOffset + ParameterCount;
    }

    /// Forward evaluation: y = Activation(W*x + b).
    /// @param weightAddr Weight address (pointer-like).
    /// @param biasAddr Bias address (pointer-like). Ignored if HasBias=false.
    /// @param input Input vector.
    /// @return Output vector after linear transform and activation.
    [Differentiable]
    [ForceInline]
    public OutputVector eval<A>(A weightAddr, A biasAddr, InputVector input)
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
        return activation.eval<OutputVector>(y);
    }
}
