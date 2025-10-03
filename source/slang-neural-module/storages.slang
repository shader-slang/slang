implementing neural;

// Storage interfaces
public interface IStorage<T> : IDifferentiablePtrType
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public associatedtype Address;

    public T read(Address address);
    public void atomicAdd(Address address, T value);
    public void write(Address address, T value);

    public static Address getOffset(Address base, int elements);
}

public interface IPointerLikeAddress<T> : IDifferentiablePtrType
{
    public __subscript(uint index) -> T { get; set; }
    public This getOffset(int elements);
    public void atomicAdd(uint index, T value);
}

public struct StructuredBufferStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public typealias Address = uint;
    public typealias BufferType = RWStructuredBuffer<T>;
    public typealias Differential = StructuredBufferStorage<T.Differential>;

    public BufferType m_buffer;

    // This the the start index of the storage object in the buffer, by default it's always 0.
    // The Storage object is instantiated at a specific index in the buffer, this address is the base address.
    public Address m_baseAddress = 0U;

    public __init(BufferType buffer, Address baseAddress = 0U)
    {
        this.m_buffer = buffer;
        this.m_baseAddress = baseAddress;
    }

    public T read(Address address)
    {
        return m_buffer[address + m_baseAddress];
    }

    public void atomicAdd(Address address, T value)
    {
        __atomic_add(m_buffer[address + m_baseAddress], value);
    }

    public void write(Address address, T value)
    {
        m_buffer[address + m_baseAddress] = value;
    }

    // [require(cpp_cuda_metal_spirv)]
    public BufferType getBufferFromAddress(Address address)
    {
        let ptr = &m_buffer[address + m_baseAddress];
        return bit_cast<BufferType>(ptr);
    }

    public static Address getOffset(Address base, int elements)
    {
        return base + elements;
    }
}

/////////////////////////////
// Bindless buffer storage //
/////////////////////////////

public struct BindlessBufferStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public struct BindlessAddress : IPointerLikeAddress<T>
    {
        public typealias Differential =
            BindlessBufferStorage<T.Differential>.BindlessAddress;

        RWStructuredBuffer<T>.Handle m_handle;
        uint m_baseIndex;

        __init(RWStructuredBuffer<T>.Handle handle, uint baseIndex = 0U)
        {
            this.m_handle = handle;
            this.m_baseIndex = baseIndex;
        }

        __subscript(uint index)->T
        {
            [nonmutating]
            get { return m_handle[m_baseIndex + index]; }

            [mutating]
            set { m_handle[m_baseIndex + index] = newValue; }
        }

        public void atomicAdd(uint index, T value)
        {
            __atomic_add(m_handle[m_baseIndex + index], value);
        }

        Address getOffset(int elements)
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

        __init(T* ptr, uint baseIndex = 0U)
        {
            this.m_ptr = ptr;
            this.m_baseIndex = baseIndex;
        }

        __subscript(uint index)->T
        {
            [nonmutating]
            get { return m_ptr[m_baseIndex + index]; }

            [mutating]
            set { m_ptr[m_baseIndex + index] = newValue; }
        }

        Address getOffset(uint elements)
        {
            uint newBaseIndex = m_baseIndex + elements;
            return Address(m_ptr, newBaseIndex);
        }

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
