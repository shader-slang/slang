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
public struct InlineVector<T, int N> : IVector<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// The differential type for automatic differentiation.
    public typealias Differential = InlineVector<T.Differential, N>;

    /// The compile-time size of the vector.
    public static const int Size = N;

    public int getCount() {return N;}
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
    public __init(T[Size] data) { this.data = data; }

    /**
    Copy constructor.
    @param[in] other The vector to copy from.
    */
    public __init(This other) { this.data = other.data; }

    public __init<InputArray : IArray<T>>(InputArray data)
    {
        static_assert(data.getCount() >= N, "The size of the input array must match the vector size");
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            this.data[i] = data[i];
    }

    /**
    Element access operator.
    @param[in] index The element index (0-based).
    @return Reference to the element at the given index.
    */
    public __subscript(int index) -> T
    {
        [ForceInline]
        [Differentiable]
        get() { return this.data[index]; }

        [ForceInline]
        [Differentiable]
        set() { this.data[index] = newValue; }
    }

    // Linear transformation without bias
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        var output = OutputVector();

        // output = W * input
        [MaxIters(OutputVector.Size)]
        for (int row = 0; row < OutputVector.Size; row++)
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
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress,
        Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        // Reuse the unbias matmul method
        OutputVector output = this.linearTransform<Address, Layout, OutputVector>(weightAddress);

        [ForceUnroll]
        for (int i = 0; i < OutputVector.Size; i++)
            output[i] = output[i] + biasAddress[i];

        return output;
    }

    // Backward of linear transformation without bias (Bindless storage)
    static public void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dparameters,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T.Differential>
    {
        // dInput = dW^T * dOutput
        This.Differential d = {};
        [MaxIters(OutputVector.Size)]
        for (int j = 0; j < OutputVector.Size; j++)
        {
            let dy = doutput[j];
            [ForceUnroll]
            for (int i = 0; i < N; i++)
            {
                T.Differential prod = T.Differential.dmul(dparameters.p[i + j * N], dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        // dW = dOutput * Input^T
        [MaxIters(OutputVector.Size)]
        for (int row = 0; row < OutputVector.Size; row++)
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
    static public void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        DifferentialPtrPair<Address> dBiasAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        // Reuse the unbias backward method
        linearTransformBwd<Address, Layout, OutputVector>(dthis, dWeightAddress, doutput);

        let biasOffset = dBiasAddress.d.getOffset(0);
        // dBias = dOutput
        [ForceUnroll]
        for (int i = 0; i < OutputVector.Size; i++)
        {
            biasOffset.atomicAdd(i, doutput[i]);
        }
    }
}
