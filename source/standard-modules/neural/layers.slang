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

Parameters are packed as a contiguous block in storage:

- **weights:** `Out * In` scalars, row-major by output row: `W[row * In + col]`
- **bias (optional):** `Out` scalars immediately following weights
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
        return baseOffset + ParameterCount;
    }

    /// Forward evaluation: y = Activation(W*x + b).
    /// @param input Input vector.
    /// @param parameterAddr Base address of contiguous parameter block (weights then bias).
    /// @return Output vector after linear transform and activation.
    [Differentiable]
    [ForceInline]
    public OutputVector eval<A>(InputVector input, A parameterAddr)
        where A : IPointerLikeAddress<T>
        where A.Differential : IPointerLikeAddress<T.Differential>
    {
        OutputVector y;
        if(HasBias)
        {
            let biasAddr = parameterAddr.getOffset(OutputVector.Size * InputVector.Size);
            y = input.linearTransform<A, Layout, OutputVector>(parameterAddr, biasAddr);
        }
        else
        {
            y = input.linearTransform<A, Layout, OutputVector>(parameterAddr);
        }
        return activation.eval<OutputVector>(y);
    }
}
