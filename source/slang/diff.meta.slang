
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
    associatedtype Differential;

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

__generic<let N:int>
extension vector<float, N> : IDifferentiable
{
    typedef vector<float, N> Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return vector<float, N>(0.f);
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

    /// Pair type that serves to wrap the primal and
    /// differential types of an arbitrary type T.
__generic<T : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairType))
struct __DifferentialPair
{

    __intrinsic_op($(kIROp_MakeDifferentialPair))
    __init(T _primal, T.Differential _differential);

    __intrinsic_op($(kIROp_DifferentialPairGetDifferential))
    T.Differential d();

    T.Differential getDifferential()
    {
        return d();
    }

    __intrinsic_op($(kIROp_DifferentialPairGetPrimal))
    T p();

    T getPrimal()
    {
        return p();
    }
};

typealias IDFloat = IFloat & IDifferentiable;

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result

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
    __DifferentialPair<T> d_exp(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
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
    __DifferentialPair<T> d_sin(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
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
    __DifferentialPair<T> d_cos(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
            cos(dpx.p()),
            T.dmul(-sin(dpx.p()), dpx.d()));
    }

    __generic<let N : int>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [ForwardDerivative(d_exp_vector)]
    vector<float, N> exp(vector<float, N> x)
    {
        VECTOR_MAP_UNARY(float, N, dstd.exp, x);
    }

    __generic<let N : int>
    __DifferentialPair<vector<float, N>> d_exp_vector(__DifferentialPair<vector<float, N>> dpx)
    {
        vector<float, N> result;
        vector<float, N>.Differential d_result;
        for(int i = 0; i < N; ++i)
        { 
            __DifferentialPair<float> dpexp = dstd.d_exp(__DifferentialPair<float>(dpx.p()[i], dpx.d()[i]));
            result[i] = dpexp.p(); 
            d_result[i] = dpexp.d(); 
        }

        return __DifferentialPair<vector<float, N>>(result, d_result);
    }

};
