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

        internal RWStructuredBuffer<T>.Handle handle;
        internal uint baseIndex;

        public __init(RWStructuredBuffer<T>.Handle handle)
        {
            this.handle = handle;
            this.baseIndex = 0;
        }

        public __subscript(uint index)->T
        {
            [nonmutating]
            get { return handle[baseIndex + index]; }

            [mutating]
            set { handle[baseIndex + index] = newValue; }
        }

        [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
        [require(spvBindlessTextureNV)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(handle[baseIndex + index], value);
        }

        public Address getOffset(int elements)
        {
            uint newBaseIndex = baseIndex + elements;

            Address address = Address(handle);
            address.baseIndex = newBaseIndex;
            return address;
        }
    }
    public typealias Address = BindlessAddress;
    public typealias Differential = BindlessBufferStorage<T.Differential>;

    // Following method will not be needed for bindless storage
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) { return base.getOffset(elements); }
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

        T* ptr;

        public __init(T* ptr)
        {
            this.ptr = ptr;
        }

        public __subscript(uint index)->T
        {
            [nonmutating]
            get { return ptr[index]; }

            [mutating]
            set { ptr[index] = newValue; }
        }

        public Address getOffset(int elements)
        {
            return Address(ptr + elements);
        }

        [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
        [require(spvBindlessTextureNV)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(ptr[index], value);
        }
    }

    public typealias Address = PointerAddress;
    public typealias Differential = PointerStorage<T.Differential>;

    // Following method will not be needed for pointer storage
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) {return base.getOffset(elements);}
}
