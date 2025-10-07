implementing neural;

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
    where T.Differential : __BuiltinFloatingPointType
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
    Atomically adds a value to storage (for gradient accumulation).
    @param[in] address The address to add to.
    @param[in] value The value to add.
    */
    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    public void atomicAdd(Address address, T value);

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
@see `BindlessBufferStorage.BindlessAddress`
@category neural
*/
public interface IPointerLikeAddress<T> : IDifferentiablePtrType
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
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
    [require(spvBindlessTextureNV)]
    public void atomicAdd(uint index, T value);
}

/**
GPU structured buffer storage implementation.
Implements IStorage using GPU structured buffers (RWStructuredBuffer).
Supports offset-based addressing and atomic operations for gradient accumulation.
@param T The element type (must be a floating-point type).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@category neural
*/
public struct StructuredBufferStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    /// Address type is a simple unsigned integer index.
    public typealias Address = uint;

    /// The underlying buffer type.
    public typealias BufferType = RWStructuredBuffer<T>;

    /// Differential type for automatic differentiation.
    public typealias Differential = StructuredBufferStorage<T.Differential>;

    /// The underlying GPU buffer.
    public BufferType m_buffer;

    /**
    Base address offset within the buffer.
    @remarks Allows multiple storage instances to share the same buffer at different offsets.
    Default is 0 (start of buffer).
    */
    public Address m_baseAddress = 0U;

    /**
    Constructor.
    @param[in] buffer The underlying structured buffer.
    @param[in] baseAddress Optional base address offset (default: 0).
    */
    public __init(BufferType buffer, Address baseAddress = 0U)
    {
        this.m_buffer = buffer;
        this.m_baseAddress = baseAddress;
    }

    /**
    Reads a value from the buffer.
    @param[in] address The address relative to base address.
    @return The value at the specified address.
    */
    public T read(Address address)
    {
        return m_buffer[address + m_baseAddress];
    }

    /**
    Atomically adds a value (for gradient accumulation).
    @param[in] address The address relative to base address.
    @param[in] value The value to add.
    */
    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    public void atomicAdd(Address address, T value)
    {
        __target_switch
        {
        case hlsl: atomicAddForHLSL(address, value);
        default: __atomic_add(m_buffer[address + m_baseAddress], value);
        }
    }

    /**
    Atomic add implementation for HLSL using InterlockedCompareExchangeFloatBitwise.
    Uses compare-and-swap loop to atomically add floating-point values.
    @param[in] address The address relative to base address.
    @param[in] value The value to add.
    @remarks This method uses a compare-and-swap loop which may have performance
    implications under high contention. Only required for HLSL targets.
    Requires Shader Model 6.6 or higher for `InterlockedCompareExchangeFloatBitwise` support.
    */
    [require(hlsl, sm_6_6)]
    internal void atomicAddForHLSL(Address address, T value)
    {
        T compareValue;
        uint bufferIndex = address + m_baseAddress;

        // Compare-and-swap loop
        bool success = false;
        do
        {
            compareValue = m_buffer[bufferIndex];
            T newValue = compareValue + value;
            success = __atomic_compare_exchange(m_buffer[bufferIndex], compareValue, newValue) == compareValue;
        } while (!success);
    }

    /**
    Writes a value to the buffer.
    @param[in] address The address relative to base address.
    @param[in] value The value to write.
    */
    public void write(Address address, T value)
    {
        m_buffer[address + m_baseAddress] = value;
    }

    /**
    Gets a buffer reference from an address (for advanced use cases).
    @param[in] address The address to get buffer from.
    @return A buffer reference starting at the specified address.
    */
    // [require(cpp_cuda_metal_spirv)]
    public BufferType getBufferFromAddress(Address address)
    {
        let ptr = &m_buffer[address + m_baseAddress];
        return bit_cast<BufferType>(ptr);
    }

    /**
    Computes an offset address.
    @param[in] base The base address.
    @param[in] elements Number of elements to offset by.
    @return The offset address.
    */
    public static Address getOffset(Address base, int elements)
    {
        return base + elements;
    }
}


/**
Bindless buffer storage implementation using buffer handles.
Provides pointer-like addressing through buffer handles, enabling more flexible
memory access patterns suitable for dynamic or sparse data structures.
@param T The element type (must be a floating-point type).
@remarks Type constraints:
- `T` must conform to `__BuiltinFloatingPointType` (float, double, half, etc.)
- `T.Differential` must conform to `__BuiltinFloatingPointType` for automatic differentiation
@category neural
*/
public struct BindlessBufferStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    /**
    Bindless address type with pointer-like semantics.
    Wraps a buffer handle and base index to provide array-like access.
    */
    public struct BindlessAddress : IPointerLikeAddress<T>
    {
        public typealias Differential =
            BindlessBufferStorage<T.Differential>.BindlessAddress;

        internal RWStructuredBuffer<T>.Handle m_handle;
        internal uint m_baseIndex;

        public __init(RWStructuredBuffer<T>.Handle handle, uint baseIndex = 0U)
        {
            this.m_handle = handle;
            this.m_baseIndex = baseIndex;
        }

        public __subscript(uint index)->T
        {
            [nonmutating]
            get { return m_handle[m_baseIndex + index]; }

            [mutating]
            set { m_handle[m_baseIndex + index] = newValue; }
        }

        [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
        [require(spvBindlessTextureNV)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(m_handle[m_baseIndex + index], value);
        }

        public Address getOffset(int elements)
        {
            uint newBaseIndex = m_baseIndex + elements;
            return Address(m_handle, newBaseIndex);
        }
    }
    public typealias Address = BindlessAddress;
    public typealias Differential = BindlessBufferStorage<T.Differential>;

    // Following method will not be needed for bindless storage
    public T read(Address address) {static_assert(false, "Not implemented"); return {}; }
    public void atomicAdd(Address address, T value) {static_assert(false, "Not implemented");}
    public void write(Address address, T value) {static_assert(false, "Not implemented");}
    public static Address getOffset(Address base, int elements) { return Address(base.m_handle, base.m_baseIndex + elements); }
}

// [require(cpp_cuda_metal_spirv)]
public struct PointerStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public struct PointerAddress : IPointerLikeAddress<T>
    {
        public typealias Differential =
            PointerStorage<T.Differential>.PointerAddress;

        T* m_ptr;
        uint m_baseIndex;

        public __init(T* ptr, uint baseIndex = 0U)
        {
            this.m_ptr = ptr;
            this.m_baseIndex = baseIndex;
        }

        public __subscript(uint index)->T
        {
            [nonmutating]
            get { return m_ptr[m_baseIndex + index]; }

            [mutating]
            set { m_ptr[m_baseIndex + index] = newValue; }
        }

        public Address getOffset(uint elements)
        {
            uint newBaseIndex = m_baseIndex + elements;
            return Address(m_ptr, newBaseIndex);
        }

        [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(m_ptr[m_baseIndex + index], value);
        }
    }

    public typealias Address = PointerAddress;
    public typealias Differential = PointerStorage<T.Differential>;

    // Following method will not be needed for pointer storage
    public T read(Address address) {static_assert(false, "Not implemented"); return {}; }
    public void atomicAdd(Address address, T value) {static_assert(false, "Not implemented");}
    public void write(Address address, T value) {static_assert(false, "Not implemented");}
    public static Address getOffset(Address base, int elements) {return Address(base.m_ptr, base.m_baseIndex + elements * sizeof(T));}
}
