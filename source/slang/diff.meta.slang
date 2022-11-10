/// Modifer to mark a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

// Custom Forward Derivative Function reference
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivative(function)] : ForwardDerivativeAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivativeOf(function)] : ForwardDerivativeOfAttribute;

__attributeTarget(DeclBase)
attribute_syntax [DerivativeMember(memberName)] : DerivativeMemberAttribute;


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

#define VECTOR_MAP_D_UNARY(TYPE, COUNT, D_FUNC, VALUE) \
    vector<TYPE, COUNT> result; \
    vector<TYPE, COUNT>.Differential d_result; \
    for (int i = 0; i < N; ++i) \
    { \
        DifferentialPair<TYPE> dp_elem = D_FUNC(DifferentialPair<TYPE>(VALUE.p[i], __slang_noop_cast<TYPE.Differential>(VALUE.d[i]))); \
        result[i] = dp_elem.p; \
        d_result[i] = __slang_noop_cast<TYPE>(dp_elem.d); \
    } \
    return DifferentialPair<vector<TYPE, COUNT>>(result, d_result)


// Detach and set derivatives to zero

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(detach)]
DifferentialPair<T> __d_detach(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        dpx.p,
        T.dzero()
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(detach)]
DifferentialPair<vector<T, N>> __d_detach_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_detach, dpx);
}

// Natural Exponent

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(exp)]
DifferentialPair<T> __d_exp(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        exp(dpx.p),
        T.dmul(exp(dpx.p), dpx.d));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(exp)]
DifferentialPair<vector<T, N>> __d_exp_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_exp, dpx);
}

// Absolute value

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(abs)]
DifferentialPair<T> __d_abs(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        abs(dpx.p),
        dpx.p > T(0.0) ? dpx.d : T.dmul(T(-1.0), dpx.d)
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(abs)]
DifferentialPair<vector<T, N>> __d_abs_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_abs, dpx);
}

// Sine

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

/*


    // Extract sign of value
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl, "int(sign($0))")
    __target_intrinsic(cuda, "$P_sign($0)")
    __target_intrinsic(cpp, "$P_sign($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(6,7) _0")
    [ForwardDerivative(d_sign<T>)]
    T sign(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_sign(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            sign(dpx.p()),
            T.dzero()
        );
    }

    __generic<let N : int>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(6,7) _0")
    [ForwardDerivative(d_sign_vector)]
    vector<float, N> sign(vector<float, N> x)
    {
        VECTOR_MAP_UNARY(N, dstd.sign, x);
    }

    __generic<let N : int>
    DifferentialPair<vector<float, N>> d_sign_vector(DifferentialPair<vector<float, N>> dpx)
    {
        VECTOR_MAP_D_UNARY(N, dstd.d_sign, dpx);
    }



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

    __generic<let N : int>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [ForwardDerivative(d_exp_vector)]
    vector<float, N> exp(vector<float, N> x)
    {
        VECTOR_MAP_UNARY(N, dstd.exp, x);
    }

    __generic<let N : int>
    DifferentialPair<vector<float, N>> d_exp_vector(DifferentialPair<vector<float, N>> dpx)
    {
        VECTOR_MAP_D_UNARY(N, dstd.d_exp, dpx);
    }

    // Base-e logarithm
    // TODO: vectorize
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_log($0)")
    __target_intrinsic(cpp, "$P_log($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 28 _0")
    [ForwardDerivative(d_log<T>)]
    T log(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_log(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            log(dpx.p()),
            T.dmul(T(1.0) / dpx.p(), dpx.d())
        );
    }

    // Sine
    // TODO: vectorize
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
    // TODO: vectorize
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

    // Sqrt root
    // TODO: vectorize
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_sqrt($0)")
    __target_intrinsic(cpp, "$P_sqrt($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 31 _0")
    [ForwardDerivative(d_sqrt<T>)]
    T sqrt(T x);

    __generic<T : IDFloat>
    DifferentialPair<T> d_sqrt(DifferentialPair<T> dpx)
    {
        // Special case

        //if (dpx.p() < T(1e-6f))
        //{
        //    return DifferentialPair<T>(T(0.0), T.dzero());
        //}

        T val = sqrt(dpx.p());
        return DifferentialPair<T>(
            val,
            T.dmul(T(0.5) / val, dpx.d())
        );
    }

    // Maximum
    // TODO: vectorize
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_max($0, $1)")
    __target_intrinsic(cpp, "$P_max($0, $1)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(40,41,42) _0")
    [ForwardDerivative(d_max<T>)]
    T max(T x, T y);

    __generic<T : IDFloat>
    DifferentialPair<T> d_max(DifferentialPair<T> dpx, DifferentialPair<T> dpy)
    {
        //// x.p > y.p ? x.d : y.d <==> 0.5 * ((x.d + y.d) + sign(x.p - y.p) * (x.d - y.d))
        T.Differential d_res = T.dmul(sign(dpx.p() - dpy.p()), T.dadd(dpx.d(), T.dmul(T(-1.0), dpy.d())));
        d_res = T.dadd(T.dadd(dpx.d(), dpy.d()), d_res);

        return DifferentialPair<T>(
            max(dpx.p(), dpy.p()),
            T.dmul(T(0.5), d_res)
            //dpx.p() > dpy.p() ? dpx.d() : dpy.d()
        );
    }

    // Raise to a power
    // TODO: vectorize
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_pow($0, $1)")
    __target_intrinsic(cpp, "$P_pow($0, $1)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 26 _0 _1")
    [ForwardDerivative(d_pow<T>)]
    T pow(T x, T y);

    __generic<T : IDFloat>
    DifferentialPair<T> d_pow(DifferentialPair<T> dpx, DifferentialPair<T> dpy)
    {
        // Special case
        //if (dpx.p() < T(1e-6f))
        //{
        //    return DifferentialPair<T>(T(0.0), T.dzero());
        //}

        T val = pow(dpx.p(), dpy.p());
        T.Differential d1 = T.dmul(val * log(dpx.p()), dpy.d());
        T.Differential d2 = T.dmul(val * dpy.p() / dpx.p(), dpx.d());
        return DifferentialPair<T>(
            val,
            T.dadd(d1, d2)
        );
    }

    // Vector dot product
    __generic<let N : int>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    [ForwardDerivative(d_dot<N>)]
    float dot(vector<float, N> x, vector<float, N> y)
    {
        float result = 0.f;
        for (int i = 0; i < N; ++i)
            result = result + x[i] * y[i];
        return result;
    }

    __generic<let N : int>
    DifferentialPair<float> d_dot(DifferentialPair<vector<float, N>> dpx, DifferentialPair<vector<float, N>> dpy)
    {
        float result = 0.f;
        float.Differential d_result = float.dzero();
        for (int i = 0; i < N; ++i)
        {
            result = result + dpx.p()[i] * dpy.p()[i];
            d_result = d_result + dpx.p()[i] * dpy.d()[i] + dpx.d()[i] * dpy.p()[i];
        }
        return DifferentialPair<float>(result, d_result);
    }
};
    return DifferentialPair<T>(
        exp(dpx.p),
        T.dmul(exp(dpx.p), dpx.d));
}
*/
