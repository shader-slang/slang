/// Modifer to mark a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

// Custom Forward Derivative Function reference
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivative(function)] : ForwardDerivativeAttribute;

/// Interface to denote types as differentiable.
/// Allows for user-specified differential types as
/// well as automatic generation, for when the associated type
/// hasn't been declared explicitly.
/// Note that the requirements must currently be defined in this exact order
/// since the auto-diff pass relies on the order to grab the struct keys.
///
__magic_type(DifferentiableType)
interface IDifferentiable
{
    // Note: the compiler implementation requires the `Differential` associated type to be defined
    // before anything else.

    [__BuiltinRequirement(_BuiltinRequirementKind.DifferentialType)]
    associatedtype Differential;

    [__BuiltinRequirement(_BuiltinRequirementKind.DZeroFunc)]
    static Differential dzero();

__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivativeOf(function)]   : ForwardDerivativeOfAttribute;

__attributeTarget(DeclBase)
attribute_syntax [DerivativeMember(memberName)] : DerivativeMemberAttribute;

// Add extensions for the standard types
extension float : IDifferentiable
{
    typedef float Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return float(0.f);
    }

/// Pair type that serves to wrap the primal and
/// differential types of an arbitrary type T.

__generic<T : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairType))
struct DifferentialPair : IDifferentiable
{
    typedef DifferentialPair<T.Differential> Differential;
    typedef T.Differential DifferentialElementType;

    __intrinsic_op($(kIROp_MakeDifferentialPair))
    __init(T _primal, T.Differential _differential);

    property p : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
        get;
    }

    property v : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
        get;
    }

    property d : T.Differential
    {
        __intrinsic_op($(kIROp_DifferentialPairGetDifferential))
        get;
    }

    [__unsafeForceInlineEarly]
    static __DifferentialBottom dmul(This a, Differential b)
    {
        return dzero();
    }
}

/// Pair type that serves to wrap the primal and
/// differential types of an arbitrary type T.
__generic<T : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairType))
struct DifferentialPair : IDifferentiable
{
    typedef DifferentialPair<T.Differential> Differential;
    typedef T.Differential DifferentialElementType;

    __intrinsic_op($(kIROp_MakeDifferentialPair))
    __init(T _primal, T.Differential _differential);

    property p : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
        get;
    }

    property v : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
        get;
    }

    property d : T.Differential
    {
        __intrinsic_op($(kIROp_DifferentialPairGetDifferential))
        get;
    }

    [__unsafeForceInlineEarly]
    T.Differential getDifferential()
    {
        return d;
    }

    [__unsafeForceInlineEarly]
    T getPrimal()
    {
        return p;
    }

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return Differential(T.dzero(), T.Differential.dzero());
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return Differential(
            T.dadd(
                a.p,
                b.p
            ),
            T.Differential.dadd(a.d, b.d));
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return Differential(
            T.dmul(a.p, b.p),
            T.Differential.dmul(a.d, b.d));
    }
};

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result

// Natural Exponent

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(exp)]
DifferentialPair<T> __d_exp(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        exp(dpx.p),
        T.dmul(exp(dpx.p), dpx.d));
}

__generic<T:__BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(exp)]
DifferentialPair<vector<T, N>> __d_exp_vector(DifferentialPair<vector<T, N>> dpx)
{
    vector<T, N> result;
    vector<T, N>.Differential d_result;
    for(int i = 0; i < N; ++i)
    {
        DifferentialPair<T> dpexp = __d_exp(DifferentialPair<T>(dpx.p[i], __slang_noop_cast<T.Differential>(dpx.d[i])));
        result[i] = dpexp.p;
        d_result[i] = __slang_noop_cast<T>(dpexp.d);
    }
    return DifferentialPair<vector<T, N>>(result, d_result);
}

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(sin)]
DifferentialPair<T> d_sin(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        sin(dpx.p),
        T.dmul(cos(dpx.p), dpx.d));
}

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(cos)]
DifferentialPair<T> d_cos(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        cos(dpx.p),
        T.dmul(-sin(dpx.p), dpx.d));
}
