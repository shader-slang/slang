implementing neural;

/**
Concrete implementation of IVector storing elements inline (on stack/registers).
InlineVector stores all elements in a fixed-size array, making it suitable for
small vectors that can fit in registers or stack memory. Supports automatic differentiation
for gradient computation in neural networks.
@param T The element type
@param N The vector size (compile-time constant).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@category neural
*/
public struct InlineVector<T, int N> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// The differential type for automatic differentiation.
    public typealias Differential = InlineVector<T.Differential, N>;

    /// The compile-time size of the vector.
    public static const int Size = N;

    /**
    Internal storage for vector elements.
    @remarks Marked as derivative member to enable automatic differentiation.
    */
    [DerivativeMember(Differential.data)]
    internal T[N] data;

    /// Default constructor - initializes all elements to zero.
    public __init() { data = {}; }

    /**
    Scalar broadcast constructor - fills all elements with the same value.
    @param[in] value The value to broadcast to all elements.
    */
    public __init(T value) {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            data[i] = value;
    }

    /**
    Array constructor - initializes from an array.
    @param[in] data Array of N elements to initialize the vector.
    */
    public __init(T[N] data) { this.data = data; }

    /**
    Copy constructor.
    @param[in] other The vector to copy from.
    */
    public __init(This other) { this.data = other.data; }

    /**
    Element access operator.
    @param[in] index The element index (0-based).
    @return Reference to the element at the given index.
    */
    public __subscript(int index) -> T
    {
        get() { return this.data[index]; }
        set() { this.data[index] = newValue; }
    }

    // Linear transformation without bias
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        no_diff Storage.Address weightAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();

        [MaxIters(OutputSize)]
        for (int row = 0; row < OutputSize; row++)
        {
            // get the address of each row of the weight matrix
            let rowOffset = Storage.getOffset(weightAddress, row * N);
            [ForceUnroll]
            for (int col = 0; col < N; col++)
            {
                let elementOffset = Storage.getOffset(rowOffset, col);
                output[row] += data[col] * weightStorage.read(elementOffset);
            }
        }

        return output;
    }

    // Linear transformation with bias
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        Storage biasStorage,
        no_diff Storage.Address weightAddress,
        no_diff Storage.Address biasAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        // Reuse the unbias matmul method
        OutputVector output = this.linearTransform<OutputSize, Storage, OutputVector>(weightStorage, weightAddress);

        // apply the bias
        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
        {
            let elementOffset = Storage.getOffset(biasAddress, i);
            output[i] = output[i] + biasStorage.read(elementOffset);
        }

        return output;
    }

    // Backward of linear transformation without bias
    static void linearTransformBwd<int OutputSize, Storage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dWeightStorage,
        no_diff Storage.Address dWeightAddress,
        OutputVector.Differential doutput)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        // Derivative of the input is transposed weight matrix times the output differential
        var d = dthis.d;

        [MaxIters(OutputSize)]
        for (int j = 0; j < OutputSize; j++)
        {
            T.Differential dy = doutput[j];

            [ForceUnroll]
            for (int i = 0; i < N; i++)
            {
                Storage.Address elementOffset = Storage.getOffset(dWeightAddress, i * OutputSize + j);
                T.Differential prod = T.Differential.dmul(dWeightStorage.p.read(elementOffset), dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        // dW = dOutput * dInput^T
        [MaxIters(OutputSize)]
        for (int row = 0; row < OutputSize; row++)
        {
            let rowOffset = Storage.getOffset(dWeightAddress, row * N);
            T.Differential dy = doutput[row];
            [ForceUnroll]
            for (int col = 0; col < N; col++)
            {
                let x = dthis.p[col];
                let elementOffset = Storage.getOffset(rowOffset, col);
                T.Differential prod = T.Differential.dmul(x, dy);
                dWeightStorage.d.atomicAdd(elementOffset, prod);
            }
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }

    // Backward of linear transformation with bias
    static void linearTransformBwd<int OutputSize, Storage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dWeightStorage,
        DifferentialPtrPair<Storage> dBiasStorage,
        no_diff Storage.Address dWeightAddress,
        no_diff Storage.Address dBiasAddress,
        OutputVector.Differential doutput)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        // Reuse the unbias backward method
        linearTransformBwd<OutputSize, Storage, OutputVector>(dthis, dWeightStorage, dWeightAddress, doutput);

        // Derivative of the bias is the same as the output differential
        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
        {
            let biasOffset = Storage.getOffset(dBiasAddress, i);
            dBiasStorage.d.atomicAdd(biasOffset, doutput[i]);
        }
    }

    // Linear transformation without bias (Bindless storage)
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(
        Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        var output = OutputVector();

        // output = W * input
        [MaxIters(OutputSize)]
        for (int row = 0; row < OutputSize; row++)
        {
            // get the address of each row of the weight matrix
            let rowAddr = weightAddress.getOffset(row * N);
            [ForceUnroll]
            for (int col = 0; col < N; col++)
            {
                output[row] += data[col] * rowAddr[col];
            }
        }
        return output;
    }

    // Linear transformation with bias (Bindless storage)
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(
        Address weightAddress,
        Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        // Reuse the unbias matmul method
        OutputVector output = this.linearTransform<OutputSize, Address, OutputVector>(weightAddress);

        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
            output[i] = output[i] + biasAddress[i];

        return output;
    }

    // Backward of linear transformation without bias (Bindless storage)
    static public void linearTransformBwd<int OutputSize, Address, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dparameters,
        OutputVector.Differential doutput)
      where Address : IPointerLikeAddress<T>
      where Address.Differential : IPointerLikeAddress<T.Differential>
      where OutputVector : IVector<T, OutputSize>
      where OutputVector.Differential : IVector<T.Differential, OutputSize>
    {
        // dInput = dW^T * dOutput
        var d = dthis.d;
        [MaxIters(OutputSize)]
        for (int j = 0; j < OutputSize; j++)
        {
            let dy = doutput[j];
            [ForceUnroll]
            for (int i = 0; i < N; i++)
            {
                T.Differential prod = T.Differential.dmul(dparameters.p[i * OutputSize + j], dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        // dW = dOutput * dInput^T
        [MaxIters(OutputSize)]
        for (int row = 0; row < OutputSize; row++)
        {
            let rowAddr = dparameters.d.getOffset(row * N);
            T.Differential dy = doutput[row];
            [ForceUnroll]
            for (int col = 0; col < N; col++)
            {
                let x = dthis.p[col];
                T.Differential prod = T.Differential.dmul(x, dy);
                rowAddr.atomicAdd(col, prod);
            }
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }

    // Backward of linear transformation with bias (Bindless storage)
    static public void linearTransformBwd<int OutputSize, Address, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        DifferentialPtrPair<Address> dBiasAddress,
        OutputVector.Differential doutput)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T.Differential>
        where OutputVector : IVector<T, OutputSize>
    {
        // Reuse the unbias backward method
        linearTransformBwd<OutputSize, Address, OutputVector>(dthis, dWeightAddress, doutput);

        let biasOffset = dBiasAddress.d.getOffset(0);
        // dBias = dOutput
        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
        {
            biasOffset.atomicAdd(i, doutput[i]);
        }
    }
}
