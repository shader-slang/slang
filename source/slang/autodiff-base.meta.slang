// Eagerly loaded autodiff declarations.
//
// Keep declarations here when ordinary user code can name them directly, or when semantic
// checking needs them without first encountering a differentiability attribute or a
// differentiation expression. Derivative implementation machinery belongs in diff.meta.slang,
// which is loaded only after one of those explicit autodiff triggers.

// A 'none-type' that acts as a run-time sentinel for zero differentials. This is primarily
// for internal use.
//
[__AutoDiffBuiltin]
[KnownBuiltin($((int)KnownBuiltinDeclName::NullDifferential))]
struct NullDifferential : IDifferentiable
{
    // for now, we'll use at least one field to make sure the type is non-empty
    float dummy;
    typedef NullDifferential Differential;

    [Differentiable]
    [ForceInline]
    static Differential dzero() { return { 0.0f }; }

    [Differentiable]
    [ForceInline]
    static Differential dadd(Differential, Differential) { return { 0.0f }; }
};

/// Represents a GPU view of a tensor.
__generic<T>
__magic_type(TensorViewType)
__intrinsic_type($(kIROp_TensorViewType))
struct TensorView
{
    [__NoSideEffect]
    [require(cuda)]
    Ptr<T> data_ptr()
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.data_ptr<$[0]>()", T;
        }
    }

    [__NoSideEffect]
    [require(cuda)]
    Ptr<T> data_ptr_at(uint index)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.data_ptr_at<$[0]>($1)", T;
        }
    }

    __generic<let N : int>
    [__NoSideEffect]
    [require(cuda)]
    Ptr<T> data_ptr_at(vector<uint, N> index)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.data_ptr_at<$[0]>($1)", T;
        }
    }

    __implicit_conversion($(kConversionCost_ImplicitDereference))
    __intrinsic_op($(kIROp_TorchTensorGetView))
    __init(TorchTensor<T> t);

    [__NoSideEffect]
    [require(cuda)]
    T load(uint x)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$[0]>($1)", T;
        }
    }

    [__NoSideEffect]
    [require(cuda)]
    T load(uint x, uint y)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$[0]>($1, $2)", T;
        }
    }

    [__NoSideEffect]
    [require(cuda)]
    T load(uint x, uint y, uint z)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$[0]>($1, $2, $3)", T;
        }
    }

    [__NoSideEffect]
    [require(cuda)]
    T load(uint x, uint y, uint z, uint w)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$[0]>($1, $2, $3, $4)", T;
        }
    }

    [__NoSideEffect]
    [require(cuda)]
    T load(uint i0, uint i1, uint i2, uint i3, uint i4)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$[0]>($1, $2, $3, $4, $5)", T;
        }
    }

    __generic<let N : int>
    [__NoSideEffect]
    [require(cuda)]
    T load(vector<uint, N> index)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.load<$TR>($1)";
        }
    }

    [require(cuda)]
    void store(uint x, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T2>($1, $2)";
        }
    }

    [require(cuda)]
    void store(uint x, uint y, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T3>($1, $2, $3)";
        }
    }

    [require(cuda)]
    void store(uint x, uint y, uint z, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T4>($1, $2, $3, $4)";
        }
    }

    [require(cuda)]
    void store(uint x, uint y, uint z, uint w, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T5>($1, $2, $3, $4, $5)";
        }
    }

    [require(cuda)]
    void store(uint i0, uint i1, uint i2, uint i3, uint i4, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T6>($1, $2, $3, $4, $5, $6)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void store(vector<uint, N> index, T val)
    {
        __target_switch
        {
        case cuda:
            __intrinsic_asm "$0.store<$T2>($1, $2)";
        }
    }

    [require(cuda)]
    void InterlockedAdd(uint index, T val, out T oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicAdd($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N:int>
    [require(cuda)]
    void InterlockedAdd(vector<uint, N> index, T val, out T oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicAdd($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [__readNone]
    [require(cuda)]
    uint dims()
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "$0.dimensionCount";
        }
    }

    [__readNone]
    [require(cuda)]
    uint size(uint i)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "$0.sizes[$1]";
        }
    }

    [__readNone]
    [require(cuda)]
    uint stride(uint i)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "$0.strides[$1]";
        }
    }

    __subscript(uint index) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(index); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(index, newValue);
        }

        [__NoSideEffect]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1)", T;
            }
        }
    }
    __subscript(uint i1, uint i2) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(i1, i2); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i1, i2, newValue);
        }

        [__NoSideEffect]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1, $2)", T;
            }
        }
    }
    __subscript(uint2 i) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(i.x, i.y); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i.x, i.y, newValue);
        }

        [__NoSideEffect]
        [nonmutating]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1.x, $1.y)", T;
            }
        }
    }
    __subscript(uint i1, uint i2, uint i3) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(i1, i2, i3); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i1, i2, i3, newValue);
        }

        [__NoSideEffect]
        [nonmutating]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1, $2, $3)", T;
            }
        }
    }
    __subscript(uint3 i) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(i.x, i.y, i.z); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i.x, i.y, i.z, newValue);
        }

        [__NoSideEffect]
        [nonmutating]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1.x, $1.y, $1.z)", T;
            }
        }
    }
    __subscript(uint i1, uint i2, uint i3, uint i4) -> T
    {
        [ForceInline]
        [__NoSideEffect]
        get { return load(i1, i2, i3, i4); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i1, i2, i3, i4, newValue);
        }

        [__NoSideEffect]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1, $2, $3, $4)", T;
            }
        }
    }
    __subscript(uint4 i) -> T
    {
        [__NoSideEffect][ForceInline] get { return load(i.x, i.y, i.z, i.w); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i.x, i.y, i.z, i.w, newValue);
        }

        [__NoSideEffect]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1.x, $1.y, $1.z, $1.w)", T;
            }
        }
    }
    __subscript(uint i1, uint i2, uint i3, uint i4, uint i5) -> T
    {
        [ForceInline] [__NoSideEffect] get { return load(i1, i2, i3, i4, i5); }

        [ForceInline]
        [nonmutating]
        set
        {
            store(i1, i2, i3, i4, i5, newValue);
        }

        [__NoSideEffect]
        [nonmutating]
        ref
        {
            __target_switch
            {
            case cuda:
                __intrinsic_asm "$0.load<$[0]>($1, $2, $3, $4, $5)", T;
            }
        }
    }
}

${
for (auto atomicIntegerTypeName : kCudaAtomicIntegerTypes)
{
$}
extension TensorView<$(atomicIntegerTypeName)>
{
    typealias __Element = $(atomicIntegerTypeName);

    [require(cuda)]
    void InterlockedMin(uint index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicMin($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedMin(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicMin($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedMax<T>(uint index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicMax($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedMax(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicMax($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedAnd<T>(uint index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicAnd($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedAnd(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicAnd($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedOr<T>(uint index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicOr($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedOr(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicOr($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedXor<T>(uint index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicXor($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedXor(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicXor($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedExchange(uint index, __Element va, out __Element oldVall)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicExch($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    __generic<let N:int>
    [require(cuda)]
    void InterlockedExchange(vector<uint, N> index, __Element val, out __Element oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicExch($0.data_ptr_at<$T2>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedCompareExchange(uint index, __Element compare, __Element val)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "atomicCAS($0.data_ptr_at<$T2>($1), $2, $3)";
        }
    }

    __generic<let N:int>
    [require(cuda)]
    void InterlockedCompareExchange(vector<uint, N> index, __Element compare, __Element val)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "atomicCAS($0.data_ptr_at<$T2>($1), $2, $3)";
        }
    }
}

${
} // end for atomicIntegerTypeName
$}

extension TensorView<float>
{
    [require(cuda)]
    float InterlockedExchange(uint index, float val, out float oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicExch($0.data_ptr_at<float>($1), $2)";
        }
    }

    __generic<let N:int>
    [require(cuda)]
    float InterlockedExchange(vector<uint, N> index, float val, out float oldVal)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "*($3) = atomicExch($0.data_ptr_at<float>($1), $2)";
        }
    }

    [require(cuda)]
    void InterlockedCompareExchange(uint index, float compare, float val)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "atomicCAS($0.data_ptr_at<uint32_t>($1), slang_bit_cast<uint32_t>($2), slang_bit_cast<uint32_t>($3))";
        }
    }

    __generic<let N : int>
    [require(cuda)]
    void InterlockedCompareExchange(vector<uint, N> index, float compare, float val)
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "atomicCAS($0.data_ptr_at<uint32_t>($1), slang_bit_cast<uint32_t>($2), slang_bit_cast<uint32_t>($3))";
        }
    }
}

interface IDiffTensorWrapper
{
    // Derivatives for universal load/store operations.

    __generic<T : __BuiltinFloatingPointType>
    T load_forward(uint i);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T load_forward(vector<uint, N> i);

    __generic<T : __BuiltinFloatingPointType>
    void load_backward(uint i, T dOut);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void load_backward(vector<uint, N> i, T dOut);

    __generic<T : __BuiltinFloatingPointType>
    void store_forward(uint i, T dx);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void store_forward(vector<uint, N> i, T dx);

    __generic<T : __BuiltinFloatingPointType>
    T store_backward(uint i);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T store_backward(vector<uint, N> i);

    // Derivatives for loadOnce/storeOnce operations. These operations
    // are designed to only run once per-address and don't need atomic
    // gradient handling.
    //

    __generic<T : __BuiltinFloatingPointType>
    T loadOnce_forward(uint i);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T loadOnce_forward(vector<uint, N> i);

    __generic<T : __BuiltinFloatingPointType>
    void loadOnce_backward(uint i, T dOut);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void loadOnce_backward(vector<uint, N> i, T dOut);

    __generic<T : __BuiltinFloatingPointType>
    void storeOnce_forward(uint i, T dx);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void storeOnce_forward(vector<uint, N> i, T dx);

    __generic<T : __BuiltinFloatingPointType>
    T storeOnce_backward(uint i);

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T storeOnce_backward(vector<uint, N> i);
};

struct AtomicAdd : IDiffTensorWrapper
{
    TensorView<float> diff;

    // Derivatives for universal load/store operations.

    __generic<T : __BuiltinFloatingPointType>
    T load_forward(uint i)
    {
        return __realCast<T, float>(diff.load(i));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T load_forward(vector<uint, N> i)
    {
        return __realCast<T, float>(diff.load(i));
    }

    __generic<T : __BuiltinFloatingPointType>
    void load_backward(uint i, T dOut)
    {
        float oldVal;
        diff.InterlockedAdd(i, __realCast<float, T>(dOut), oldVal);
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void load_backward(vector<uint, N> i, T dOut)
    {
        float oldVal;
        diff.InterlockedAdd(i, __realCast<float, T>(dOut), oldVal);
    }

    __generic<T : __BuiltinFloatingPointType>
    void store_forward(uint i, T dx)
    {
        diff.store(i, __realCast<float, T>(dx));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void store_forward(vector<uint, N> i, T dx)
    {
        diff.store(i, __realCast<float, T>(dx));
    }

    __generic<T : __BuiltinFloatingPointType>
    T store_backward(uint i)
    {
        float oldVal;
        diff.InterlockedExchange(i, (float)0, oldVal);
        return __realCast<T, float>(oldVal);
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T store_backward(vector<uint, N> i)
    {
        float oldVal;
        diff.InterlockedExchange(i, (float)0, oldVal);
        return __realCast<T, float>(oldVal);
    }

    // Derivatives for loadOnce/storeOnce operations. These operations
    // are designed to only run once per-address and don't need atomic
    // gradient handling.
    //

    __generic<T : __BuiltinFloatingPointType>
    T loadOnce_forward(uint i)
    {
        return __realCast<T, float>(diff.load(i));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T loadOnce_forward(vector<uint, N> i)
    {
        return __realCast<T, float>(diff.load(i));
    }

    __generic<T : __BuiltinFloatingPointType>
    void loadOnce_backward(uint i, T dOut)
    {
        diff.store(i, __realCast<float, T>(dOut));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void loadOnce_backward(vector<uint, N> i, T dOut)
    {
        diff.store(i, __realCast<float, T>(dOut));
    }

    __generic<T : __BuiltinFloatingPointType>
    void storeOnce_forward(uint i, T dx)
    {
        diff.store(i, __realCast<float, T>(dx));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    void storeOnce_forward(vector<uint, N> i, T dx)
    {
        diff.store(i, __realCast<float, T>(dx));
    }

    __generic<T : __BuiltinFloatingPointType>
    T storeOnce_backward(uint i)
    {
        return __realCast<T, float>(diff.load(i));
    }

    __generic<T : __BuiltinFloatingPointType, let N : int>
    T storeOnce_backward(vector<uint, N> i)
    {
        return __realCast<T, float>(diff.load(i));
    }
};

__generic<T: __BuiltinFloatingPointType = float, A : IDiffTensorWrapper = AtomicAdd>
struct DiffTensorView
{
    TensorView<T> primal;
    A diff;

    uint size(uint i)
    {
        return primal.size(i);
    }

    uint dims()
    {
        return primal.dims();
    }

    uint stride(uint i)
    {
        return primal.stride(i);
    }

    // Constructors
    __init(TensorView<T> primal, A diff)
    {
        this.primal = primal;
        this.diff = diff;
    }

    __init(TensorView<T> primal)
    {
        this.primal = primal;
    }

    // Universal load/store operations.

    [BackwardDerivative(__load_backward)]
    [ForwardDerivative(__load_forward)]
    [NoDiffThis]
    T load(uint i) { return primal.load(i); }

    [BackwardDerivative(__load_backward)]
    [ForwardDerivative(__load_forward)]
    [NoDiffThis]
    __generic<let N : int>
    T load(vector<uint, N> i) { return primal.load(i); }

    DifferentialPair<T> __load_forward(uint x)
    {
        return diffPair(primal.load(x), reinterpret<T.Differential, T>(diff.load_forward<T>(x)));
    }

    [ForceInline]
    __generic<let N : int>
    DifferentialPair<T> __load_forward(vector<uint, N> x)
    {
        return diffPair(primal.load(x), reinterpret<T.Differential, T>(diff.load_forward<T, N>(x)));
    }

    [ForceInline]
    void __load_backward(uint x, T.Differential dOut)
    {
        diff.load_backward<T>(x, reinterpret<T, T.Differential>(dOut));
    }

    [ForceInline]
    __generic<let N : int>
    void __load_backward(vector<uint, N> x, T.Differential dOut)
    {
        diff.load_backward<T, N>(x, reinterpret<T, T.Differential>(dOut));
    }

    [BackwardDerivative(__store_backward)]
    [ForwardDerivative(__store_forward)]
    [NoDiffThis]
    void store(uint x, T val) { primal.store(x, val); }

    [BackwardDerivative(__store_backward)]
    [ForwardDerivative(__store_forward)]
    [NoDiffThis]
    __generic<let N : int>
    void store(vector<uint, N> x, T val) { primal.store(x, val); }

    void __store_forward(uint x, DifferentialPair<T> dpval)
    {
        primal.store(x, dpval.p);
        diff.store_forward<T>(x, reinterpret<T, T.Differential>(dpval.d));
    }

    __generic<let N : int>
    void __store_forward(vector<uint, N> x, DifferentialPair<T> dpval)
    {
        primal.store(x, dpval.p);
        diff.store_forward<T, N>(x, reinterpret<T, T.Differential>(dpval.d));
    }

    [ForceInline]
    void __store_backward(uint x, inout DifferentialPair<T> dpval)
    {
        dpval = diffPair(dpval.p, reinterpret<T.Differential, T>(diff.store_backward<T>(x)));
    }

    [ForceInline]
    __generic<let N : int>
    void __store_backward(vector<uint, N> x, inout DifferentialPair<T> dpval)
    {
        dpval = diffPair(dpval.p, reinterpret<T.Differential, T>(diff.store_backward<T, N>(x)));
    }

    __subscript(uint index)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(index); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(index, newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint2 index)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(index); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(index, newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint x, uint y)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(uint2(x, y)); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(uint2(x, y), newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint3 index)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(index); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(index, newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint x, uint y, uint z)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(uint3(x, y, z)); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(uint3(x, y, z), newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint4 index)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(index); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(index, newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    __subscript(uint x, uint y, uint z, uint w)->T
    {
        [__unsafeForceInlineEarly] [Differentiable] [__NoSideEffect] get { return load(uint4(x, y, z, w)); }
        [__unsafeForceInlineEarly] [Differentiable] [nonmutating] set { store(uint4(x, y, z, w), newValue); }

        [__NoSideEffect]
        [nonmutating]
        ref;
    }

    // loadOnce/storeOnce operations. These operations are designed to only run once per-address and
    // don't need atomic gradient handling.
    //

    [BackwardDerivative(__loadOnce_backward)]
    [ForwardDerivative(__loadOnce_forward)]
    [NoDiffThis]
    T loadOnce(uint i) { return primal.load(i); }

    [BackwardDerivative(__loadOnce_backward)]
    [ForwardDerivative(__loadOnce_forward)]
    [NoDiffThis]
    __generic<let N : int>
    T loadOnce(vector<uint, N> i) { return primal.load(i); }

    DifferentialPair<T> __loadOnce_forward(uint x)
    {
        return diffPair(primal.load(x), reinterpret<T.Differential, T>(diff.loadOnce_forward<T>(x)));
    }

    __generic<let N : int>
    DifferentialPair<T> __loadOnce_forward(vector<uint, N> x)
    {
        return diffPair(primal.load(x), reinterpret<T.Differential, T>(diff.loadOnce_forward<T, N>(x)));
    }

    [ForceInline]
    void __loadOnce_backward(uint x, T.Differential dOut)
    {
        diff.loadOnce_backward<T>(x, reinterpret<T, T.Differential>(dOut));
    }

    [ForceInline]
    __generic<let N : int>
    void __loadOnce_backward(vector<uint, N> x, T.Differential dOut)
    {
        diff.loadOnce_backward<T, N>(x, reinterpret<T, T.Differential>(dOut));
    }

    [BackwardDerivative(__storeOnce_backward)]
    [ForwardDerivative(__storeOnce_forward)]
    [NoDiffThis]
    void storeOnce(uint x, T val) { primal.store(x, val); }

    [BackwardDerivative(__storeOnce_backward)]
    [ForwardDerivative(__storeOnce_forward)]
    [NoDiffThis]
    __generic<let N : int>
    void storeOnce(vector<uint, N> x, T val) { primal.store(x, val); }

    void __storeOnce_forward(uint x, DifferentialPair<T> dpval)
    {
        primal.store(x, dpval.p);
        diff.storeOnce_forward<T>(x, reinterpret<T, T.Differential>(dpval.d));
    }

    __generic<let N : int>
    void __storeOnce_forward(vector<uint, N> x, DifferentialPair<T> dpval)
    {
        primal.store(x, dpval.p);
        diff.storeOnce_forward<T, N>(x, reinterpret<T, T.Differential>(dpval.d));
    }

    [ForceInline]
    void __storeOnce_backward(uint x, inout DifferentialPair<T> dpval)
    {
        dpval = diffPair(dpval.p, reinterpret<T.Differential, T>(diff.storeOnce_backward<T>(x)));
    }

    [ForceInline]
    __generic<let N : int>
    void __storeOnce_backward(vector<uint, N> x, inout DifferentialPair<T> dpval)
    {
        dpval = diffPair(dpval.p, reinterpret<T.Differential, T>(diff.storeOnce_backward<T, N>(x)));
    }
};

/// Represents the handle of a Torch tensor object.
__generic<T>
__intrinsic_type($(kIROp_TorchTensorType))
struct TorchTensor
{
    __intrinsic_op($(kIROp_TorchTensorGetView))
    [CudaHost]
    TensorView<T> getView();

    [__readNone]
    [CudaHost]
    [require(cpp_cuda)]
    uint dims()
    {
        __target_switch
        {
        case cpp: __intrinsic_asm "$0.dims()";
        case cuda: __intrinsic_asm "$0.dims()";
        }
    }

    [__readNone]
    [CudaHost]
    [require(cpp_cuda)]
    uint size(uint i)
    {
        __target_switch
        {
        case cpp: __intrinsic_asm "$0.size($1)";
        case cuda: __intrinsic_asm "$0.size($1)";
        }
    }

    [__readNone]
    [CudaHost]
    [require(cpp_cuda)]
    uint stride(uint i)
    {
        __target_switch
        {
        case cpp: __intrinsic_asm "$0.stride($1)";
        case cuda: __intrinsic_asm "$0.stride($1)";
        }
    }

    [__readNone]
    [CudaHost]
    [require(cpp_cuda)]
    Ptr<T> data_ptr()
    {
        __target_switch
        {
        case cpp:
        case cuda:
            __intrinsic_asm "$0.data_ptr<$[0]>()", T;
        }
    }

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> alloc(uint x);

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> alloc(uint x, uint y);

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> alloc(uint x, uint y, uint z);

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> alloc(uint x, uint y, uint z, uint w);

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> alloc(uint i0, uint i1, uint i2, uint i3, uint i4);

    __intrinsic_op($(kIROp_AllocateTorchTensor))
    [CudaHost]
    static TorchTensor<T> emptyLike(TorchTensor<T> other);

    [CudaHost]
    [require(cpp)]
    void fillZero()
    {
        __target_switch
        {
        case cpp: __intrinsic_asm "$0.zero_()";
        }
    }

    [CudaHost]
    [require(cpp)]
    void fillValue(T val)
    {
        __target_switch
        {
        case cpp: __intrinsic_asm "$0.fill_($1)";
        }
    }

    [CudaHost]
    static TorchTensor<T> zerosLike(TorchTensor<T> other)
    {
        var result = emptyLike(other);
        result.fillZero();
        return result;
    }

}

[require(cpp)]
void syncTorchCudaStream()
{
    __target_switch
    {
    case cpp: __intrinsic_asm "AT_CUDA_CHECK(cudaStreamSynchronize(at::cuda::getCurrentCUDAStream()))";
    }
}

[__unsafeForceInlineEarly]
void updatePrimal<T : IDifferentiable>(inout DifferentialPair<T> p, T newPrimal)
{
    p = DifferentialPair<T>(newPrimal, p.d);
}

[__unsafeForceInlineEarly]
void updateDiff<T : IDifferentiable>(inout DifferentialPair<T> p, T.Differential newDiff)
{
    p = DifferentialPair<T>(p.p, newDiff);
}

[__unsafeForceInlineEarly]
void updatePair<T : IDifferentiable>(inout DifferentialPair<T> p, T newPrimal, T.Differential newDiff)
{
    p = DifferentialPair<T>(newPrimal, newDiff);
}

/// Detach and set derivatives to zero.
__generic<T : IDifferentiable>
__intrinsic_op($(kIROp_DetachDerivative))
T detach(T x);

__generic<T, let N:int>
__intrinsic_op($(kIROp_MakeArrayFromElement))
Array<T,N> makeArrayFromElement(T element);


__generic<T:IDifferentiable, let N:int>
extension Array<T, N> : IDifferentiable
{
    typedef Array<T.Differential, N> Differential;

    [Differentiable]
    static Differential dzero()
    {
        return makeArrayFromElement<T.Differential, N>(T.dzero());
    }

    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        Array<T.Differential, N> result;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            result[i] = T.dadd(a[i], b[i]);
        return result;
    }
}

__generic<T : IDifferentiablePtrType, let N : int>
extension Array<T, N> : IDifferentiablePtrType
{
    typedef Array<T.Differential, N> Differential;
}

__generic<T:IDifferentiable>
extension Optional<T> : IDifferentiable
{
    typedef Optional<T.Differential> Differential;

    [__unsafeForceInlineEarly]
    [Differentiable]
    static Differential dzero()
    {
        return Optional<T.Differential>();
    }

    [__unsafeForceInlineEarly]
    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        if (!a.hasValue)
            return b;
        if (b.hasValue)
            return T.dadd(a.value, b.value);
        else
            return a;
    }
}

__generic<T : IDifferentiablePtrType>
extension Optional<T> : IDifferentiablePtrType
{
    typedef Optional<T.Differential> Differential;
}

__generic<each T : IDifferentiable>
extension Tuple<T> : IDifferentiable
{
    typealias Differential = Tuple<expand(each T).Differential>;

    [__unsafeForceInlineEarly]
    [Differentiable]
    static Differential dzero()
    {
        return makeTuple(expand (each T).dzero());
    }

    [__unsafeForceInlineEarly]
    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        return makeTuple(expand(each T).dadd(each a, each b));
    }
}
