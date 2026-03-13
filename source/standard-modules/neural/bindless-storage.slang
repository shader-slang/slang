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

    [ForceInline]
    [require(hlsl, sm_6_6)]
    internal void atomicAddForHLSL(uint index, T value)
    {
        T compareValue;
        bool success = false;
        do
        {
            compareValue = handle[baseIndex + index];
            T newValue = compareValue + value;
            success = __atomic_compare_exchange(handle[baseIndex + index], compareValue, newValue) == compareValue;
        } while (!success);
    }

    [ForceInline]
    public void atomicAdd(uint index, T value)
    {
        __target_switch
        {
        case hlsl:
            atomicAddForHLSL(index, value);
        default:
            __atomic_reduce_add(handle[baseIndex + index], value);
        }
    }

    [ForceInline]
    public void atomicAdd(uint index, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            // On CUDA, use packed vector atomic for types that support it (half2, bfloat16x2).
            // Note: Only use this on CUDA, because Ptr<T[]> is just T** on CUDA.

            // *handle will be resolved to RWStructuredBuffer<T>
            let bufferPtr = __getStructuredBufferPtr(*handle);
            let doublePtr = reinterpret<T**>(bufferPtr);
            let offsetPtr = doublePtr[0] + baseIndex + index;
            let vecPtr = reinterpret<vector<T, 2>*>(offsetPtr);
            __atomic_reduce_add(*vecPtr, value);
        default:
            // On other targets, fall back to two scalar atomic adds.
            atomicAdd(index, value[0]);
            atomicAdd(index + 1, value[1]);
        }
    }

    [Differentiable]
    [ForwardDerivative(fwd_getOffset)]
    public This getOffset(int elements)
    {
        uint newBaseIndex = baseIndex + elements;

        This address = This(handle);
        address.baseIndex = newBaseIndex;
        return address;
    }

    static DifferentialPtrPair<This> fwd_getOffset(DifferentialPtrPair<This> self, int elements)
    {
        return DifferentialPtrPair<This>(
            self.p.getOffset(elements),
            self.d.getOffset(elements));
    }

    typealias BufferType = RWStructuredBuffer<T>;

    [ForceInline]
    internal RWStructuredBuffer<T> getBuffer()
    {
        return *handle;
    }

    [ForceInline]
    internal uint getBaseIndex()
    {
        return baseIndex;
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

    [Differentiable]
    [ForwardDerivative(fwd_getOffset)]
    public This getOffset(int elements)
    {
        return no_diff(This(ptr + elements));
    }

    static DifferentialPtrPair<This> fwd_getOffset(DifferentialPtrPair<This> self, int elements)
    {
        return DifferentialPtrPair<This>(
            self.p.getOffset(elements),
            self.d.getOffset(elements));
    }

    typealias BufferType = T*;

    [ForceInline]
    internal T* getBuffer()
    {
        return ptr;
    }

    [ForceInline]
    internal uint getBaseIndex()
    {
        return 0;
    }

    [ForceInline]
    [require(hlsl, sm_6_6)]
    internal void atomicAddForHLSL(uint index, T value)
    {
        T compareValue;
        bool success = false;
        do
        {
            compareValue = ptr[index];
            T newValue = compareValue + value;
            success = __atomic_compare_exchange(ptr[index], compareValue, newValue) == compareValue;
        } while (!success);
    }

    [ForceInline]
    public void atomicAdd(uint index, T value)
    {
        __target_switch
        {
        case hlsl:
            atomicAddForHLSL(index, value);
        default:
            __atomic_reduce_add(ptr[index], value);
        }
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

/**
TensorView address type with pointer-like semantics for slangtorch integration.
Wraps a TensorView by extracting its data pointer into a PointerAddress, delegating
all IPointerLikeAddress operations to it.
@param T The element type.
@remarks CUDA-only, since TensorView is a CUDA-specific type.
@category neural
*/

public struct TorchTensorViewAddress<T> : IPointerLikeAddress<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = TorchTensorViewAddress<T.Differential>;

    internal PointerAddress<T> inner;

    [require(cuda)]
    public __init(TensorView<T> tensor, uint baseAddress = 0U)
    {
        this.inner = PointerAddress<T>(tensor.data_ptr() + baseAddress);
    }

    [require(cuda)]
    public __subscript(uint index)->T
    {
        [nonmutating] get { return inner[index]; }
        [mutating] set { inner[index] = newValue; }
    }

    [require(cuda)]
    [Differentiable]
    [ForwardDerivative(fwd_getOffset)]
    public This getOffset(int elements)
    {
        This result;
        result.inner = inner.getOffset(elements);
        return result;
    }

    static DifferentialPtrPair<This> fwd_getOffset(DifferentialPtrPair<This> self, int elements)
    {
        return DifferentialPtrPair<This>(
            self.p.getOffset(elements),
            self.d.getOffset(elements));
    }

    typealias BufferType = T*;

    [ForceInline]
    internal T* getBuffer()
    {
        return inner.getBuffer();
    }

    [ForceInline]
    internal uint getBaseIndex()
    {
        return inner.getBaseIndex();
    }

    [ForceInline]
    public void atomicAdd(uint index, T value)
    {
        __target_switch
        {
        case cuda:
            inner.atomicAdd(index, value);
        default:
            static_assert(false, "atomicAdd is not supported on this target");
        }
    }

    [ForceInline]
    public void atomicAdd(uint index, vector<T, 2> value) {
        __target_switch
        {
        case cuda:
            inner.atomicAdd(index, value);
        default:
            static_assert(false, "atomicAdd is not supported on this target");
        }
    }

    [ForceInline]
    internal uint4 readUint4<DstType, bool IsAligned, uint ActualBoundary>(int offsetIndex)
        where DstType : __BuiltinFloatingPointType
        where DstType.Differential == DstType
    {
        return inner.readUint4<DstType, IsAligned, ActualBoundary>(offsetIndex);
    }

    [ForceInline]
    internal void writeUint4Atomic<SrcType, bool IsAligned, uint ActualBoundary>(int offsetIndex, uint4 value)
        where SrcType : __BuiltinFloatingPointType
        where SrcType.Differential == SrcType
    {
        inner.writeUint4Atomic<SrcType, IsAligned, ActualBoundary>(offsetIndex, value);
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

    [Differentiable]
    [ForwardDerivative(fwd_getOffset)]
    internal This getOffset(int elements)
    {
        return no_diff(This(this + elements));
    }

    static DifferentialPtrPair<This> fwd_getOffset(DifferentialPtrPair<This> self, int elements)
    {
        return DifferentialPtrPair<This>(
            self.p + elements,
            self.d + elements);
    }

    typealias BufferType = T*;

    [ForceInline]
    internal T* getBuffer()
    {
        return this + 0u;
    }

    [ForceInline]
    internal uint getBaseIndex()
    {
        return 0;
    }

    [require(hlsl, sm_6_6)]
    internal void atomicAddForHLSL(uint index, T value)
    {
        T compareValue;
        bool success = false;
        do
        {
            compareValue = this[index];
            T newValue = compareValue + value;
            success = __atomic_compare_exchange(this[index], compareValue, newValue) == compareValue;
        } while (!success);
    }

    [ForceInline]
    [ForceInline]
    internal void atomicAdd(uint index, T value)
    {
        __target_switch
        {
        case hlsl:
            atomicAddForHLSL(index, value);
        default:
            __atomic_reduce_add(this[index], value);
        }
    }

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

// ---------------------------------------------------------------------------
// Cooperative-matrix load/store free functions.
//
// Workaround: ideally these would be constrained extensions with
//   where Address.BufferType == RWStructuredBuffer<T>
//   where Address.BufferType == T*
// but associated-type equality constraints in where clauses are not yet
// supported (https://github.com/shader-slang/slang/issues/9622).
// Once that is fixed, replace with constrained extensions so the compiler
// resolves CoopMat.Load/Store overloads statically.
// ---------------------------------------------------------------------------

[ForceInline]
[require(cooperative_matrix)]
internal linalg.CoopMat<T, MemoryScope.Subgroup, M, N, R>
    loadCoopMat<T, Address, let M : int, let N : int, let R : linalg.CoopMatMatrixUse, let Layout : linalg.CoopMatMatrixLayout>(
        Address addr, uint element, uint stride)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Address : IPointerLikeAddress<T>
{
    typealias RetType = linalg.CoopMat<T, MemoryScope.Subgroup, M, N, R>;
    if (T is half)
    {
        if (Address.BufferType is RWStructuredBuffer<half>)
        {
            return bit_cast<RetType>(
                linalg.CoopMat<half, MemoryScope.Subgroup, M, N, R>.Load<Layout>(
                    bit_cast<RWStructuredBuffer<half>>(addr.getBuffer()),
                    addr.getBaseIndex() + element, stride));
        }
        else if (Address.BufferType is half*)
        {
            return bit_cast<RetType>(
                linalg.CoopMat<half, MemoryScope.Subgroup, M, N, R>.Load<Layout>(
                    bit_cast<half*>(addr.getBuffer()),
                    addr.getBaseIndex() + element, stride));
        }
        else
        {
            static_assert(false, "Unsupported buffer type");
        }
    }
    else if (T is float)
    {
        if (Address.BufferType is RWStructuredBuffer<float>)
        {
            return bit_cast<RetType>(
                linalg.CoopMat<float, MemoryScope.Subgroup, M, N, R>.Load<Layout>(
                    bit_cast<RWStructuredBuffer<float>>(addr.getBuffer()),
                    addr.getBaseIndex() + element, stride));
        }
        else if (Address.BufferType is float*)
        {
            return bit_cast<RetType>(
                linalg.CoopMat<float, MemoryScope.Subgroup, M, N, R>.Load<Layout>(
                    bit_cast<float*>(addr.getBuffer()),
                    addr.getBaseIndex() + element, stride));
        }
        else
        {
            static_assert(false, "Unsupported buffer type");
        }
    }
    static_assert(false, "Unsupported element type");
    RetType dummy;
    return dummy;
}

[ForceInline]
[require(cooperative_matrix)]
internal void storeCoopMat<T, Address, let M : int, let N : int, let R : linalg.CoopMatMatrixUse, let Layout : linalg.CoopMatMatrixLayout>(
    Address addr,
    linalg.CoopMat<T, MemoryScope.Subgroup, M, N, R> mat,
    uint element, uint stride)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where Address : IPointerLikeAddress<T>
{
    if (T is half)
    {
        let concreteMat = bit_cast<linalg.CoopMat<half, MemoryScope.Subgroup, M, N, R>>(mat);
        if (Address.BufferType is RWStructuredBuffer<half>)
        {
            concreteMat.Store<Layout>(
                bit_cast<RWStructuredBuffer<half>>(addr.getBuffer()),
                addr.getBaseIndex() + element, stride);
        }
        else if (Address.BufferType is half*)
        {
            concreteMat.Store<Layout>(
                bit_cast<half*>(addr.getBuffer()),
                addr.getBaseIndex() + element, stride);
        }
        else
        {
            static_assert(false, "Unsupported buffer type");
        }
    }
    else if (T is float)
    {
        let concreteMat = bit_cast<linalg.CoopMat<float, MemoryScope.Subgroup, M, N, R>>(mat);
        if (Address.BufferType is RWStructuredBuffer<float>)
        {
            concreteMat.Store<Layout>(
                bit_cast<RWStructuredBuffer<float>>(addr.getBuffer()),
                addr.getBaseIndex() + element, stride);
        }
        else if (Address.BufferType is float*)
        {
            concreteMat.Store<Layout>(
                bit_cast<float*>(addr.getBuffer()),
                addr.getBaseIndex() + element, stride);
        }
        else
        {
            static_assert(false, "Unsupported buffer type");
        }
    }
}

