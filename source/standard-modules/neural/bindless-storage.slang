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

    [ForceInline]
    public void atomicAdd(uint index, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            // On CUDA, use packed vector atomic for types that support it (half2, bfloat16x2).
            let scalarPtr = &handle[baseIndex + index];
            let vecPtr = reinterpret<vector<T, 2>*>(scalarPtr);
            __atomic_reduce_add(vecPtr[0], value);
        default:
            // On other targets, fall back to two scalar atomic adds.
            atomicAdd(index, value[0]);
            atomicAdd(index + 1, value[1]);
        }
    }

    public This getOffset(int elements)
    {
        uint newBaseIndex = baseIndex + elements;

        This address = This(handle);
        address.baseIndex = newBaseIndex;
        return address;
    }

    [ForceInline]
    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(int offsetIndex)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        uint4 value;
        accessUint4<AccessOp.READ, DstType, T, RWStructuredBuffer<T>.Handle, IsAligned, ActualBoundary>(
            handle, int(baseIndex), int(baseIndex) + offsetIndex, value);
        return value;
    }

    [ForceInline]
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(int offsetIndex, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        accessUint4<AccessOp.ATOMIC_ADD, SrcType, T, RWStructuredBuffer<T>.Handle, IsAligned, ActualBoundary>(
            handle, int(baseIndex), int(baseIndex) + offsetIndex, value);
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

    [ForceInline]
    public void atomicAdd(uint index, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            // On CUDA, use packed vector atomic for types that support it (half2, bfloat16x2).
            let vecPtr = reinterpret<vector<T, 2>*>(ptr + index);
            __atomic_reduce_add(vecPtr[0], value);
        default:
            // On other targets, fall back to two scalar atomic adds.
            atomicAdd(index, value[0]);
            atomicAdd(index + 1, value[1]);
        }
    }

    [ForceInline]
    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(int offsetIndex)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        uint4 value;
        accessUint4<AccessOp.READ, DstType, T, T*, IsAligned, ActualBoundary>(
            ptr, 0, offsetIndex, value);
        return value;
    }

    [ForceInline]
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(int offsetIndex, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        accessUint4<AccessOp.ATOMIC_ADD, SrcType, T, T*, IsAligned, ActualBoundary>(
            ptr, 0, offsetIndex, value);
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

    [ForceInline]
    internal void atomicAdd(uint index, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            let vecPtr = reinterpret<vector<T, 2>*>(this + index);
            __atomic_reduce_add(vecPtr[0], value);
        default:
            atomicAdd(index, value[0]);
            atomicAdd(index + 1, value[1]);
        }
    }

    [ForceInline]
    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(int offsetIndex)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        uint4 value;
        accessUint4<AccessOp.READ, DstType, T, This, IsAligned, ActualBoundary>(
            this, 0, offsetIndex, value);
        return value;
    }

    [ForceInline]
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(int offsetIndex, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        accessUint4<AccessOp.ATOMIC_ADD, SrcType, T, This, IsAligned, ActualBoundary>(
            this, 0, offsetIndex, value);
    }
}

