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
Storage interface for accessing neural network parameters.
Provides an abstraction for reading/writing parameters from various storage backends
(GPU buffers, shared memory, etc.). Supports automatic differentiation for gradient accumulation.
@param T The element type (must be a floating-point type).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@see `StructuredBufferStorage`, `BindlessBufferStorage`
@category neural
*/
public interface IStorage<T> : IDifferentiablePtrType
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    /**
    Address type for this storage backend.
    @remarks Different storage types may use different addressing schemes (indices, pointers, etc.).
    */
    public associatedtype Address;

    /**
    Reads a value from storage.
    @param[in] address The address to read from.
    @return The value at the specified address.
    */
    public T read(Address address);

    /**
    Reads sequential 4 elements starting from the given address and packed into a uint4.
    */

    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType;

    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType;

    /**
    Atomically adds a value to storage (for gradient accumulation).
    @param[in] address The address to add to.
    @param[in] value The value to add.
    */
    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    public void atomicAdd(Address address, T value);

    /**
    Atomically adds a vector of 2 values to storage using packed atomic operations where available.
    On CUDA, this uses optimized PTX `red` instruction (e.g., red.relaxed.gpu.global.add.noftz.f16x2 for half2).
    On other targets, falls back to two scalar atomicAdd calls.
    @param[in] address The address to add to (must be aligned to 2 elements).
    @param[in] value The vector of 2 values to add.
    */
    public void atomicAdd(Address address, vector<T, 2> value);

    /**
    Writes a value to storage.
    @param[in] address The address to write to.
    @param[in] value The value to write.
    */
    public void write(Address address, T value);

    /**
    Computes an offset address.
    @param[in] base The base address.
    @param[in] elements Number of elements to offset by.
    @return The offset address.
    */
    public static Address getOffset(Address base, int elements);
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
}
