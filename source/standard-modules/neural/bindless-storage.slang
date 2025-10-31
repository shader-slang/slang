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
        __atomic_add(handle[baseIndex + index], value);
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
        __atomic_add(ptr[index], value);
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
    internal void atomicAdd(uint index, T value) {__atomic_add(this[index], value);}
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
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {address.atomicAdd(0, value);}
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
    public T read(Address address) {return address[0];}
    public void atomicAdd(Address address, T value) {address.atomicAdd(0, value);}
    public void write(Address address, T value) {address[0] = value;}
    public static Address getOffset(Address base, int elements) {return base.getOffset(elements);}
}
