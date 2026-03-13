implementing neural;

public enum LayoutType : uint32_t
{
    Linear = 0,
    Optimal = 1,
}

internal interface IStorageLayout
{
    internal static const LayoutType Layout;
}

public struct LinearLayout : IStorageLayout
{
    internal static const LayoutType Layout = LayoutType.Linear;
}

/// Storage layout where the weight matrix is pre-tiled into cooperative-matrix-sized tiles.
///
/// The M x K weight matrix is divided into ceil(M/16) x ceil(K/16) tiles of 16x16 elements.
/// **Both M and K must be multiples of 16.**
/// Tiles are stored in row-major tile order:
///   tile(tileRow, tileCol) at linear index = tileRow * numTileCols + tileCol
/// Within each tile, elements are stored contiguously in row-major order:
///   element(r, c) at offset r * 16 + c within the tile.
///
/// This layout enables loading weight tiles directly without shared-memory staging
/// or zero-padding, and eliminates cross-warp synchronisation for matrix A.
public struct OptimalLayout : IStorageLayout
{
    internal static const LayoutType Layout = LayoutType.Optimal;
}

/**
Interface for pointer-like addressing with direct subscript access.
Provides array-like access patterns for storage backends that support pointer arithmetic.
@param T The element type.
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@see `BindlessBufferStorage.BindlessAddress`
@category neural
*/
/// Sealed: implementations are provided by the neural module (PointerAddress,
/// TorchTensorViewAddress, etc.). User code should use these existing types
/// rather than implementing the interface directly.
[sealed]
public interface IPointerLikeAddress<T> : IDifferentiablePtrType
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /**
    Array-style element access.
    @param[in] index The element index.
    @return Reference to the element at the given index.
    */
    public __subscript(uint index) -> T { get; set; }

    /**
    Computes an offset pointer.
    @param[in] elements Number of elements to offset by.
    @return The offset pointer.
    */
    public This getOffset(int elements);

    /**
    Atomically adds a value at an index.
    @param[in] index The element index.
    @param[in] value The value to add.
    */
    public void atomicAdd(uint index, T value);

    /**
    Atomically adds a vector of 2 values using packed atomic operations where available.
    @param[in] index The element index (must be aligned to 2 elements).
    @param[in] value The vector of 2 values to add.
    */
    public void atomicAdd(uint index, vector<T, 2> value);

    /// The underlying buffer type used by this address for cooperative matrix loads.
    /// Buffer-backed addresses (e.g. BindlessAddress) use `RWStructuredBuffer<T>`,
    /// which generates `OpAccessChain` on SPIR-V and avoids variable pointer requirements.
    /// Pointer-backed addresses use `T*`.
    associatedtype BufferType;

    /// Returns the underlying buffer or pointer that backs this address.
    [ForceInline]
    internal BufferType getBuffer();

    /// Returns the current element offset within the buffer.
    /// For pointer-based addresses where the offset is baked into the pointer, returns 0.
    [ForceInline]
    internal uint getBaseIndex();

    /**
    Reads sequential elements starting from the given offset address, packed into a uint4.
    */
    [ForceInline]
    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(int offsetIndex)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType;

    /**
    Atomically writes unpacked uint4 values back to storage.
    */
    [ForceInline]
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(int offsetIndex, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType;

}
