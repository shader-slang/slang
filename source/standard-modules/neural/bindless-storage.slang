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

        public __init(RWStructuredBuffer<T>.Handle handle, uint baseIndex = 0U)
        {
            this.handle = handle;
            this.baseIndex = baseIndex;
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
            return Address(handle, newBaseIndex);
        }
    }
    public typealias Address = BindlessAddress;
    public typealias Differential = BindlessBufferStorage<T.Differential>;

    // Following method will not be needed for bindless storage
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) { return Address(base.handle, base.baseIndex + elements); }
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
        uint baseIndex;

        public __init(T* ptr, uint baseIndex = 0U)
        {
            this.ptr = ptr;
            this.baseIndex = baseIndex;
        }

        public __subscript(uint index)->T
        {
            [nonmutating]
            get { return ptr[baseIndex + index]; }

            [mutating]
            set { ptr[baseIndex + index] = newValue; }
        }

        public Address getOffset(uint elements)
        {
            uint newBaseIndex = baseIndex + elements;
            return Address(ptr, newBaseIndex);
        }

        [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
        [require(spvBindlessTextureNV)]
        public void atomicAdd(uint index, T value)
        {
            __atomic_add(ptr[baseIndex + index], value);
        }
    }

    public typealias Address = PointerAddress;
    public typealias Differential = PointerStorage<T.Differential>;

    // Following method will not be needed for pointer storage
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) {return Address(base.ptr, base.baseIndex + elements * sizeof(T));}
}
