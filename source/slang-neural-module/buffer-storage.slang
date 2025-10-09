implementing neural;

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
    where T.Differential == T
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
