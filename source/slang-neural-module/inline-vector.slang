implementing neural;

/**
Generic vector interface for neural network operations.
Provides a differentiable vector abstraction supporting automatic differentiation
and linear algebra operations for neural network computations.
@param T The element type (must be a floating-point type).
@param N The vector size (compile-time constant).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@see `InlineVector`
@category neural
*/
public interface IVector<T, int N> : IDifferentiable
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    /// The compile-time size of the vector.
    public static const int Size;

    /// The differential type for automatic differentiation.
    /// @remarks Ensures the differential is also a vector with the same structure.
    public associatedtype Differential : IVector<T.Differential, N>;

    /// Default constructor - initializes vector to zero.
    public __init();

    /**
    Scalar broadcast constructor - fills all elements with the same value.
    @param[in] value The value to broadcast to all elements.
    */
    public __init(T value);

    /**
    Array constructor - initializes from an array.
    @param[in] data Array of N elements to initialize the vector.
    */
    public __init(T[N] data);

    /**
    Copy constructor.
    @param[in] other The vector to copy from.
    */
    public __init(This other);

    /**
    Element access operator.
    @param[in] index The element index (0-based).
    @return Reference to the element at the given index.
    */
    public __subscript(int index)->T
    {
        get;
        set;
    }

    /**
    Evaluates a linear transformation: output = W * input (+ bias).
    Performs matrix-vector multiplication with optional bias addition.
    Supports automatic differentiation for backpropagation.
    @param OutputSize The size of the output vector.
    @param Bias Whether to include bias in the computation.
    @param Storage The storage type for parameters (must implement IStorage).
    @param OutputVector The output vector type.
    @param[in] storage The storage object containing weight matrix and optional bias.
    @param[in] address The starting address in storage for the weight matrix.
    @return The result of the linear transformation.
    @remarks Type constraints:
    - `Storage` must conform to `IStorage<T>`
    - `Storage.Differential` must conform to `IStorage<T.Differential>`
    - `Storage.Address` must equal `Storage.Differential.Address` (same address type)
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [BackwardDifferentiable]
    public OutputVector mad<int OutputSize, bool Bias, Storage, OutputVector>(
        Storage storage,
        no_diff Storage.Address address)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>;

    /**
    Evaluates a linear transformation using bindless storage (pointer-like addressing).
    Similar to eval() but uses pointer-like addressing for more flexible memory access.
    @param OutputSize The size of the output vector.
    @param BindlessStorage The bindless storage type with pointer-like addresses.
    @param OutputVector The output vector type.
    @param[in] parameters The address of parameters in bindless storage.
    @return The result of the linear transformation.
    @remarks Type constraints:
    - `BindlessStorage` must conform to `IStorage<T>`
    - `BindlessStorage.Address` must conform to `IPointerLikeAddress<T>`
    - `BindlessStorage.Address.Differential` must conform to `IPointerLikeAddress<T.Differential>`
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [BackwardDifferentiable]
    public OutputVector mad<int OutputSize, BindlessStorage, OutputVector>(
        BindlessStorage.Address parameters)
            where BindlessStorage : IStorage<T>
            where BindlessStorage.Address : IPointerLikeAddress<T>
            where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>;
}

/**
Concrete implementation of IVector storing elements inline (on stack/registers).
InlineVector stores all elements in a fixed-size array, making it suitable for
small vectors that can fit in registers or stack memory. Supports automatic differentiation
for gradient computation in neural networks.
@param T The element type (must be a floating-point type).
@param N The vector size (compile-time constant).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@category neural
*/
public struct InlineVector<T, int N> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    /// The differential type for automatic differentiation.
    public typealias Differential = InlineVector<T.Differential, N>;

    /// The compile-time size of the vector.
    public static const int Size = N;

    /**
    Internal storage for vector elements.
    @remarks Marked as derivative member to enable automatic differentiation.
    */
    [DerivativeMember(Differential.m_data)]
    internal T[N] m_data;

    /// Default constructor - initializes all elements to zero.
    public __init() { m_data = {}; }

    /**
    Scalar broadcast constructor - fills all elements with the same value.
    @param[in] value The value to broadcast to all elements.
    */
    public __init(T value) {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            m_data[i] = value;
    }

    /**
    Array constructor - initializes from an array.
    @param[in] data Array of N elements to initialize the vector.
    */
    public __init(T[N] data) { this.m_data = data; }

    /**
    Copy constructor.
    @param[in] other The vector to copy from.
    */
    public __init(This other) { this.m_data = other.m_data; }

    /**
    Element access operator.
    @param[in] index The element index (0-based).
    @return Reference to the element at the given index.
    */
    public __subscript(int index) -> T
    {
        get() { return this.m_data[index]; }
        set() { this.m_data[index] = newValue; }
    }

    [BackwardDerivative(madbwd)]
    public OutputVector mad<int OutputSize, bool Bias, Storage, OutputVector>(
        Storage storage,
        no_diff Storage.Address address)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address
        where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();

        if (Bias)
        {
            let biasOffset = Storage.getOffset(address, N * OutputSize);
            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
            {
                let biasAddress = Storage.getOffset(biasOffset, i);
                output[i] = storage.read(biasAddress);
            }
        }

        // TODO: transpose if N >> OutputSize or OutputSize < 4
        let weightSliceAddress = address;
        [MaxIters(N)]
        for (int j = 0; j < N; j++)
        {
            let x = m_data[j];

            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
            {
                let elementOffset = Storage.getOffset(address, j * OutputSize + i);
                output[i] += x * storage.read(elementOffset);
            }
        }

        return output;
    }

    static void madbwd<int OutputSize, bool Bias, Storage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dstorage,
        no_diff Storage.Address address,
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
                Storage.Address elementOffset = Storage.getOffset(address, i * OutputSize + j);
                T.Differential prod = T.Differential.dmul(dstorage.p.read(elementOffset), dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
        {
            let x = dthis.p[i];
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                T.Differential dy = doutput[j];
                let elementOffset = Storage.getOffset(address, i * OutputSize + j);
                T.Differential prod = T.Differential.dmul(x, dy);
                Storage.Differential diffStorage = dstorage.d;
                diffStorage.atomicAdd(elementOffset, prod);
            }
        }

        // Derivative of the bias is the same as the output differential
        if (Bias)
        {
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                let biasOffset = Storage.getOffset(address, N * OutputSize + j);
                dstorage.d.atomicAdd(biasOffset, doutput[j]);
            }
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }

    // Apply from bindless storage
    [BackwardDerivative(madBwd)]
    public OutputVector mad<int OutputSize, BindlessStorage, OutputVector>(
        BindlessStorage.Address parameters)
            where BindlessStorage : IStorage<T>
            where BindlessStorage.Address : IPointerLikeAddress<T>
            where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        var output = OutputVector();

        let biases = parameters.getOffset(N * OutputSize);

        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
            output[i] = biases[i];

        [MaxIters(N)]
        for (int j = 0; j < N; j++)
        {
            let x = m_data[j];
            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
                output[i] += x * parameters[j * OutputSize + i];
        }

        return output;
    }

    static public void madBwd<int OutputSize, BindlessStorage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<BindlessStorage.Address> dparameters,
        OutputVector.Differential doutput)
      where BindlessStorage : IStorage<T>
      where BindlessStorage.Address : IPointerLikeAddress<T>
      where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
      where OutputVector : IVector<T, OutputSize>
      where OutputVector.Differential : IVector<T.Differential, OutputSize>
    {
        // Derivative of the input is transposed weight matrix times the output differential
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
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
        {
            let x = dthis.p[i];
            let weightSlice = dparameters.d.getOffset(i * OutputSize);
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                let dy = doutput[j];
                T.Differential prod = T.Differential.dmul(x, dy);
                weightSlice.atomicAdd(j, prod);
            }
        }

        let biasAddress = dparameters.d.getOffset(N * OutputSize);
        [ForceUnroll]
        for (int j = 0; j < OutputSize; j++)
        {
            biasAddress.atomicAdd(j, doutput[j]);
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }
}
