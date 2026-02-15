implementing neural;

/**
Bindless address type with pointer-like semantics.
Wraps a buffer handle and base index to provide array-like access.
*/
public struct BindlessAddress<T> : IPointerLikeAddress<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = BindlessAddress<T.Differential>;

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
    public void atomicAdd(uint index, T value)
    {
        __atomic_reduce_add(handle[baseIndex + index], value);
    }

    public This getOffset(int elements)
    {
        uint newBaseIndex = baseIndex + elements;

        This address = This(handle);
        address.baseIndex = newBaseIndex;
        return address;
    }
}

public struct PointerAddress<T> : IPointerLikeAddress<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = PointerAddress<T.Differential>;

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

    public This getOffset(int elements)
    {
        return This(ptr + elements);
    }

    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    public void atomicAdd(uint index, T value)
    {
        __atomic_reduce_add(ptr[index], value);
    }
}

// We currently don't support UserPointer as an `IDifferentiablePtrType`, the issue is tracked in
// https://github.com/shader-slang/slang/issues/8834.
// So we define an internal extension for now, once we can resolve the issue, we can make it public.
internal extension<T> Ptr<T, Access.ReadWrite, AddressSpace.Device> : IPointerLikeAddress<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal typealias Differential = Ptr<T.Differential, Access.ReadWrite, AddressSpace.Device>;

    internal __init(Ptr<T, Access.ReadWrite, AddressSpace.Device> ptr)
    {
        this = ptr;
    }

    internal __subscript(uint index)->T
    {
        [nonmutating]
        get { return this[index]; }

        [mutating]
        set { this[index] = newValue; }
    }

    internal This getOffset(int elements)
    {
        return This(this + elements);
    }

    [require(cuda_glsl_hlsl_metal_spirv, sm_6_6)]
    internal void atomicAdd(uint index, T value) {__atomic_reduce_add(this[index], value);}
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
    where T.Differential == T
{
    public typealias Address = BindlessAddress<T>;
    public typealias Differential = BindlessBufferStorage<T.Differential>;

    // Following method will not be needed for bindless storage
    public T read(Address address) { return address[0]; }

    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        static_assert(false, "Not implemented");
        return uint4(0);
    }

    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        static_assert(false, "Not implemented");
    }
    public void atomicAdd(Address address, T value) {address.atomicAdd(0, value);}

    [ForceInline]
    public void atomicAdd(Address address, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            // On CUDA, use packed vector atomic for types that support it (half2, bfloat16x2).
            if (let handle = address.handle as RWStructuredBuffer<T>.Handle)
            {
                let scalarPtr = &handle[address.baseIndex];
                let vecPtr = reinterpret<vector<T, 2>*>(scalarPtr);
                __atomic_reduce_add(vecPtr[0], value);
            }
        default:
            // On other targets, fall back to two scalar atomic adds.
            address.atomicAdd(0, value[0]);
            address.atomicAdd(1, value[1]);
        }
    }

    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) { return base.getOffset(elements); }
}

public struct PointerStorage<T> : IStorage<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Address = PointerAddress<T>;
    public typealias Differential = PointerStorage<T.Differential>;

    // Following method will not be needed for pointer storage
    public T read(Address address) { return address[0]; }

    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        static_assert(false, "Not implemented");
        return uint4(0);
    }
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(Address baseAddress, Address address, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        static_assert(false, "Not implemented");
    }
    public void atomicAdd(Address address, T value) {address.atomicAdd(0, value);}

    [ForceInline]
    public void atomicAdd(Address address, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            // On CUDA, use packed vector atomic for types that support it (half2, bfloat16x2).
            // address.ptr is already a T*, cast to vector<T, 2>* and dereference.
            let vecPtr = reinterpret<vector<T, 2>*>(address.ptr);
            __atomic_reduce_add(vecPtr[0], value);
        default:
            // On other targets, fall back to two scalar atomic adds.
            address.atomicAdd(0, value[0]);
            address.atomicAdd(1, value[1]);
        }
    }

    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) {return base.getOffset(elements);}
}
