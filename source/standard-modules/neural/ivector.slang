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
    where T.Differential == T
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
    Evaluates a linear transformation: output = W * input.
    Performs matrix-vector multiplication.
    Supports automatic differentiation for backpropagation.
    @param OutputSize The size of the output vector.
    @param Bias Whether to include bias in the computation.
    @param Storage The storage type for parameters (must implement IStorage).
    @param OutputVector The output vector type.
    @param[in] weightStorage The storage object containing weight matrix.
        The weight matrix is stored in a contiguous block of memory, the size
        of the weight matrix is in `OutputSize` rows, and `N` columns.
    @param[in] weightAddress The starting address in weightStorage for the weight matrix.
    @return The result of the linear transformation, `result = W * this`, whose size is `OutputSize`.
    @remarks Type constraints:
    - `Storage` must conform to `IStorage<T>`
    - `Storage.Differential` must conform to `IStorage<T.Differential>`
    - `Storage.Address` must equal `Storage.Differential.Address` (same address type)
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        no_diff Storage.Address weightAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
        where OutputVector : IVector<T, OutputSize>;

    /**
    Evaluates a linear transformation: output = W * this + bias.
    Performs matrix-vector multiplication with bias addition.
    Supports automatic differentiation for backpropagation.
    @param OutputSize The size of the output vector.
    @param Storage The storage type for parameters (must implement IStorage).
    @param OutputVector The output vector type.
    @param[in] weightStorage The storage object containing weight matrix.
            The weight matrix is stored in a contiguous block of memory, the size
            of the weight matrix is in `OutputSize` rows, and `N` columns.
    @param[in] weightAddress The starting address in weightStorage for the weight matrix.
    @param[in] biasStorage The storage object containing bias vector.
            The bias vector is stored in a contiguous block of memory, the size
            of the bias vector is `OutputSize`.
    @param[in] weightAddress The starting address in `weightStorage` for the weight matrix.
    @param[in] biasAddress The starting address in `biasStorage` for the bias vector.
    @return The result of the linear transformation, `result = W * this + bias`, whose size is `OutputSize`.
    @remarks Type constraints:
    - `Storage` must conform to `IStorage<T>`
    - `Storage.Differential` must conform to `IStorage<T.Differential>`
    - `Storage.Address` must equal `Storage.Differential.Address` (same address type)
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        Storage biasStorage,
        no_diff Storage.Address weightAddress,
        no_diff Storage.Address biasAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>;

    /**
    Evaluates a linear transformation: output = W * this.
    This is a overload method but using bindless storage (pointer-like addressing).
    @param OutputSize The size of the output vector.
    @param BindlessStorage The bindless storage type with pointer-like addresses.
    @param OutputVector The output vector type.
    @param[in] weightAddress The address of weight matrix in bindless storage.
            The weight matrix is stored in a contiguous block of memory, the size
            of the weight matrix is in `OutputSize` rows, and `N` columns. As this method take the
            bindless storage buffer, we don't need to provide the storage object.
    @return The result of the linear transformation, `result = W * this`, whose size is `OutputSize`.
    @remarks Type constraints:
    - `BindlessStorage` must conform to `IStorage<T>`
    - `BindlessStorage.Address` must conform to `IPointerLikeAddress<T>`
    - `BindlessStorage.Address.Differential` must conform to `IPointerLikeAddress<T.Differential>`
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>;

    /**
    Evaluates a linear transformation: output = W * this + bias.
    Performs matrix-vector multiplication with bias addition.
    Supports automatic differentiation for backpropagation.
    @param OutputSize The size of the output vector.
    @param BindlessStorage The bindless storage type with pointer-like addresses.
    @param OutputVector The output vector type.
    @param[in] weightAddress The address of weight matrix in bindless storage.
            The weight matrix is stored in a contiguous block of memory, the size
            of the weight matrix is in `OutputSize` rows, and `N` columns. As this method take the
            bindless storage buffer, we don't need to provide the storage object.
    @param[in] biasAddress The address of bias vector in bindless storage.
            The bias vector is stored in a contiguous block of memory, the size
            of the bias vector is `OutputSize`. As this method take the
            bindless storage buffer, we don't need to provide the storage object.
    @return The result of the linear transformation, `result = W * this + bias`, whose size is `OutputSize`.
    @remarks Type constraints:
    - `BindlessStorage` must conform to `IStorage<T>`
    - `BindlessStorage.Address` must conform to `IPointerLikeAddress<T>`
    - `BindlessStorage.Address.Differential` must conform to `IPointerLikeAddress<T.Differential>`
    - `OutputVector` must conform to `IVector<T, OutputSize>`
    */
    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(
        Address weightAddress, Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>;
}
