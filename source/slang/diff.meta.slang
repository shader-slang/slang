
/// Modifer to mark a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

// Custom Forward Derivative Function reference
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivative(function)]   : ForwardDerivativeAttribute;

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
    associatedtype Differential : IDifferentiable;

    [__BuiltinRequirement(_BuiltinRequirementKind.DZeroFunc)]
    static Differential dzero();

    [__BuiltinRequirement(_BuiltinRequirementKind.DAddFunc)]
    static Differential dadd(Differential, Differential);

    [__BuiltinRequirement(_BuiltinRequirementKind.DMulFunc)]
    static Differential dmul(This, Differential);
};

// Add extensions for the standard types
extension float : IDifferentiable
{
    typedef float Differential;
    
    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return float(0.f);
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return a * b;
    }
}

extension vector<float, 3> : IDifferentiable
{
    typedef vector<float, 3> Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return vector<float, 3>(0.f);
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return a * b;
    }
}

extension vector<float, 2> : IDifferentiable
{
    typedef vector<float, 2> Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return vector<float, 2>(0.f);
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return a * b;
    }
}

extension vector<float, 4> : IDifferentiable
{
    typedef vector<float, 4> Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return vector<float, 4>(0.f);
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return a * b;
    }
}

__magic_type(DifferentialBottomType)
__intrinsic_type($(kIROp_DifferentialBottomType))
struct __DifferentialBottom : IDifferentiable
{
    typedef __DifferentialBottom Differential;

    __intrinsic_op($(kIROp_DifferentialBottomValue))
    static __DifferentialBottom dzero();

    [__unsafeForceInlineEarly]
    static __DifferentialBottom dadd(Differential a, Differential b)
    {
        return dzero();
    }

    [__unsafeForceInlineEarly]
    static __DifferentialBottom dmul(This a, Differential b)
    {
        return dzero();
    }
}

    /// Pair type that serves to wrap the primal and
    /// differential types of an arbitrary type T.
__generic<T : IDifferentiable, TDiff : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairType))
struct __DifferentialPair : IDifferentiable
{

    typedef __DifferentialPair<TDiff, __DifferentialBottom> Differential;

    __intrinsic_op($(kIROp_MakeDifferentialPair))
    __init(T _primal, TDiff _differential);

    __intrinsic_op($(kIROp_DifferentialPairGetDifferential))
    TDiff d();

    TDiff getDifferential()
    {
        return d();
    }

    __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
    T p();

    T getPrimal()
    {
        return p();
    }

    // There are only two possible relationships between TDiff and T.Differnetial:
    // 1. TDiff == T.Differential.
    // 2. TDiff == bottom, T.Differential = someType
    // In case 1, we can trivially cast between the two.
    // In case 2, the result should yeild TargetType.dzero().
    // We implement this behavior in an IRInst DifferentialEqualityTypeCast.

    __intrinsic_op($(kIROp_DifferentialEqualityTypeCast))
    static TDiff __type_equality_cast(T.Differential d);
    __intrinsic_op($(kIROp_DifferentialEqualityTypeCast))
    static T.Differential __type_equality_cast(TDiff d);

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return Differential(__type_equality_cast(T.dzero()), __DifferentialBottom.dzero());
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return Differential(__type_equality_cast(T.dadd(__type_equality_cast(a.p()), __type_equality_cast(b.p()))), __DifferentialBottom.dzero());
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        return Differential(__type_equality_cast(T.dmul(a.p(), __type_equality_cast(b.p()))), __DifferentialBottom.dzero());
    }
};

typealias DifferentialPair<T:IDifferentiable> = __DifferentialPair<T, T.Differential>;

typealias IDFloat = IFloat & IDifferentiable;

namespace dstd
{
    // Natural Exponent
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_exp($0)")
    __target_intrinsic(cpp, "$P_exp($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [ForwardDerivative(d_exp<T>)]
    T exp(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_exp(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            exp(dpx.p()),
            T.dmul(exp(dpx.p()), dpx.d()));
    }

    // Sine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_sin($0)")
    __target_intrinsic(cpp, "$P_sin($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 13 _0")
    [ForwardDerivative(d_sin<T>)]
    T sin(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_sin(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            sin(dpx.p()),
            T.dmul(cos(dpx.p()), dpx.d()));
    }

    // Cosine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_cos($0)")
    __target_intrinsic(cpp, "$P_cos($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 14 _0")
    [ForwardDerivative(d_cos<T>)]
    T cos(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_cos(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            cos(dpx.p()),
            T.dmul(-sin(dpx.p()), dpx.d()));
    }
};
