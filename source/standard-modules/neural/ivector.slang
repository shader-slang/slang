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
public interface IVector<T> : IDifferentiable, IArrayAccessor<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /// The compile-time size of the vector.
    public static const int Size;

    /// The differential type for automatic differentiation.
    /// @remarks Ensures the differential is also a vector with the same structure.
    public associatedtype Differential : IVector<T>;

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
    public __init(T[This.Size] data);

    public __init<InputArray : IArray<T>>(InputArray data);

    /**
    Copy constructor.
    @param[in] other The vector to copy from.
    */
    public __init(This other);

    /**
    Evaluates a linear transformation: output = W * this.
    Uses pointer-like addressing for direct access to weight parameters.
    @param Address The address type with pointer-like access.
    @param Layout The storage layout type.
    @param OutputVector The output vector type.
    @param[in] weightAddress The address of the weight matrix.
            The weight matrix is stored in a contiguous block of memory, the size
            of the weight matrix is in `OutputVector.Size` rows, and `N` columns.
    @return The result of the linear transformation, `result = W * this`, whose size is `OutputVector.Size`.
    @remarks Type constraints:
    - `Address` must conform to `IPointerLikeAddress<T>`
    - `Address.Differential` must conform to `IPointerLikeAddress<T.Differential>`
    - `OutputVector` must conform to `IVector<T>`
    */
    [Differentiable]
    public OutputVector linearTransform<Address, Layout, OutputVector>(Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>;

    /**
    Evaluates a linear transformation: output = W * this + bias.
    Performs matrix-vector multiplication with bias addition.
    Uses pointer-like addressing for direct access to weight and bias parameters.
    @param Address The address type with pointer-like access.
    @param Layout The storage layout type.
    @param OutputVector The output vector type.
    @param[in] weightAddress The address of the weight matrix.
            The weight matrix is stored in a contiguous block of memory, the size
            of the weight matrix is in `OutputVector.Size` rows, and `N` columns.
    @param[in] biasAddress The address of the bias vector.
            The bias vector is stored in a contiguous block of memory, the size
            of the bias vector is `OutputVector.Size`.
    @return The result of the linear transformation, `result = W * this + bias`, whose size is `OutputVector.Size`.
    @remarks Type constraints:
    - `Address` must conform to `IPointerLikeAddress<T>`
    - `Address.Differential` must conform to `IPointerLikeAddress<T.Differential>`
    - `OutputVector` must conform to `IVector<T>`
    */
    [Differentiable]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress, Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>;
}
