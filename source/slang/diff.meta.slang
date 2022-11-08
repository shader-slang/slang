
/// Modifer to mark a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

// Custom Forward Derivative Function reference
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivative(function)]   : ForwardDerivativeAttribute;


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

namespace dstd
{
    // Natural Exponent
    __generic<T : __BuiltinFloatingPointType>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_exp($0)")
    __target_intrinsic(cpp, "$P_exp($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [ForwardDerivative(d_exp<T>)]
    T exp(T x);

    __generic<T : __BuiltinFloatingPointType>
    DifferentialPair<T> d_exp(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            dstd.exp(dpx.p),
            T.dmul(dstd.exp(dpx.p), dpx.d));
    }

    // Sine
    __generic<T : __BuiltinFloatingPointType>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_sin($0)")
    __target_intrinsic(cpp, "$P_sin($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 13 _0")
    [ForwardDerivative(d_sin<T>)]
    T sin(T x);

    __generic<T : __BuiltinFloatingPointType>
    DifferentialPair<T> d_sin(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            dstd.sin(dpx.p),
            T.dmul(dstd.cos(dpx.p), dpx.d));
    }

    // Cosine
    __generic<T : __BuiltinFloatingPointType>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_cos($0)")
    __target_intrinsic(cpp, "$P_cos($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 14 _0")
    [ForwardDerivative(d_cos<T>)]
    T cos(T x);

    __generic<T : __BuiltinFloatingPointType>
    DifferentialPair<T> d_cos(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            dstd.cos(dpx.p),
            T.dmul(-dstd.sin(dpx.p), dpx.d));
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
    DifferentialPair<vector<float, N>> d_exp_vector(DifferentialPair<vector<float, N>> dpx)
    {
        vector<float, N> result;
        vector<float, N>.Differential d_result;
        for(int i = 0; i < N; ++i)
        { 
            DifferentialPair<float> dpexp = dstd.d_exp(DifferentialPair<float>(dpx.p[i], dpx.d[i]));
            result[i] = dpexp.p; 
            d_result[i] = dpexp.d; 
        }

        return DifferentialPair<vector<float, N>>(result, d_result);
    }

};
