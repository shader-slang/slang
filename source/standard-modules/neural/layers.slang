implementing neural;

/**
A fully-connected (feed-forward) neural network layer that computes `y = Activation(W*x + b)`.

`FFLayer` represents a single linear transformation followed by an activation function,
suitable for building multi-layer perceptrons (MLPs) and similar architectures.

## Usage

1. **Construction:** Create a layer:
   ```
   let layer = FFLayer<float, Vec4, Vec2, LinearLayout, ReLU<float>>();
   ```

2. **Forward pass:** Call `eval()` with base parameter address and input:
   ```
   let output = layer.eval<Address>(input, paramAddr);
   ```

3. **Training (backward pass):** Use autodiff with `DifferentialPtrPair`:
   ```
   var addrPair = DifferentialPtrPair<Address>(paramAddr, gradAddr);
   bwd_diff(computeOutput)(addrPair, inputPair, layer, dOutput);
   ```

## Parameter Layout

`ParameterCount` reports the logical parameter count: row-major weights plus `Out` bias
scalars when `HasBias` is true. `StorageSize` reports the physical layer extent for the
selected storage layout. For `OptimalLayout`, weights are padded to a 16x16 envelope and
an aligned bias slot is reserved after the weights even when the layer has no logical bias.
*/
public struct FFLayer<
    T,
    InputVector,
    OutputVector,
    Layout,
    Activation,
    let HasBias : bool = true
>
    : ILayer<T, InputVector, OutputVector, Layout, Activation>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Layout : IStorageLayout
    where InputVector : IVector<T>
    where OutputVector : IVector<T>
    where Activation : IActivation<T>
{
    public static const int ParameterCount =
        OutputVector.Size * InputVector.Size + (HasBias ? OutputVector.Size : 0);

    static const int PaddedOutputSize = ((OutputVector.Size + 15) / 16) * 16;
    static const int PaddedInputSize = ((InputVector.Size + 15) / 16) * 16;

    // OptimalLayout uses a target-independent physical envelope: weights are
    // padded to 16x16 and bias is padded to the output extent. CUDA, Vulkan,
    // and Metal may order elements differently inside this envelope, but they
    // all reserve the same number of elements so layer offsets stay uniform.
    public static const int WeightStorageSize =
        (Layout.Layout == LayoutType.Optimal)
            ? PaddedOutputSize * PaddedInputSize
            : OutputVector.Size * InputVector.Size;

    public static const int BiasStorageSize =
        (Layout.Layout == LayoutType.Optimal)
            ? PaddedOutputSize
            : (HasBias ? OutputVector.Size : 0);

    public static const int StorageSize = WeightStorageSize + BiasStorageSize;

    /// Activation function instance (stores any activation-specific parameters).
    internal Activation activation;

    /// Constructor.
    /// @param act Activation function instance (defaults to default-constructed Activation).
    public __init(Activation act = Activation())
    {
        activation = act;
    }

    public static int nextOffset(int baseOffset)
    {
        return baseOffset + StorageSize;
    }

    /// Forward evaluation: y = Activation(W*x + b).
    /// @param input Input vector.
    /// @param parameterAddr Base address of contiguous parameter block (weights then bias).
    /// @return Output vector after linear transform and activation.
    [Differentiable]
    [ForceInline]
    public OutputVector eval<A>(InputVector input, A parameterAddr)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T>
    {
        OutputVector y;
        if(HasBias)
        {
            let biasAddr = parameterAddr.getOffset(WeightStorageSize);
            y = input.linearTransform<A, Layout, OutputVector>(parameterAddr, biasAddr);
        }
        else
        {
            y = input.linearTransform<A, Layout, OutputVector>(parameterAddr);
        }
        return activation.eval<OutputVector>(y);
    }
}
