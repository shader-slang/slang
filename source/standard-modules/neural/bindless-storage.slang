implementing neural;

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
    where T.Differential == T
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
            this.m_handle = RWStructuredBuffer<T>.Handle(handle);
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
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) { return Address(base.m_handle, base.m_baseIndex + elements); }
}

// [require(cpp_cuda_metal_spirv)]
public struct PointerStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
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
        [require(spvBindlessTextureNV)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(m_ptr[m_baseIndex + index], value);
        }
    }

    public typealias Address = PointerAddress;
    public typealias Differential = PointerStorage<T.Differential>;

    // Following method will not be needed for pointer storage
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) {return Address(base.m_ptr, base.m_baseIndex + elements * sizeof(T));}
}
