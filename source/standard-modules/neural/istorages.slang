implementing neural;

public enum LayoutType : uint32_t
{
    Linear = 0,
}

internal interface IStorageLayout
{
    internal static const LayoutType Layout;
}

public struct LinearLayout : IStorageLayout
{
    internal static const LayoutType Layout = LayoutType.Linear;
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
    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    public void atomicAdd(uint index, T value);

    /**
    Atomically adds a vector of 2 values using packed atomic operations where available.
    @param[in] index The element index (must be aligned to 2 elements).
    @param[in] value The vector of 2 values to add.
    */
    public void atomicAdd(uint index, vector<T, 2> value);

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
