/// Modifer to mark a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

// Custom Forward Derivative Function reference
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivative(function)] : ForwardDerivativeAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDerivative(function)] : BackwardDerivativeAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDifferentiable] : BackwardDifferentiableAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivativeOf(function)] : ForwardDerivativeOfAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDerivativeOf(function)] : BackwardDerivativeOfAttribute;

__attributeTarget(DeclBase)
attribute_syntax [DerivativeMember(memberName)] : DerivativeMemberAttribute;

// Exclude "this" parameter from differentiation.
__attributeTarget(FunctionDeclBase)
attribute_syntax [NoDiffThis] : NoDiffThisAttribute;


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

__generic<T: IDifferentiable>
__intrinsic_op($(kIROp_MakeDifferentialPair))
DifferentialPair<T> diffPair(T primal, T.Differential diff);

__generic<T: IDifferentiable>
[__unsafeForceInlineEarly]
DifferentialPair<T> diffPair(T primal)
{
    return diffPair(primal, T.dzero());
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

__generic<T, let N:int>
__intrinsic_op($(kIROp_MakeArrayFromElement))
Array<T,N> makeArrayFromElement(T element);


__generic<T:IDifferentiable, let N:int>
extension Array<T, N> : IDifferentiable
{
    typedef Array<T.Differential, N> Differential;

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return makeArrayFromElement<T.Differential, N>(T.dzero());
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        Array<T.Differential, N> result;
        for (int i = 0; i < N; i++)
            result[i] = T.dadd(a[i], b[i]);
        return result;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        Array<T.Differential, N> result;
        for (int i = 0; i < N; i++)
            result[i] = T.dmul(a[i], b[i]);
        return result;
    }
}

// vector-matrix
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[ForceInline]
[ForwardDerivativeOf(mul)]
DifferentialPair<vector<T, M>> mul(DifferentialPair<vector<T, N>> left, DifferentialPair<matrix<T, N, M>> right)
{
    let primal = mul(left.p, right.p);
    let diff = mul(left.d, right.p) + mul(left.p, right.d);
    return DifferentialPair<vector<T,M>>(primal, diff);
}

// matrix-vector
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[ForceInline]
[ForwardDerivativeOf(mul)]
DifferentialPair<vector<T,N>> mul(DifferentialPair<matrix<T,N,M>> left, DifferentialPair<vector<T,M>> right)
{
    let primal = mul(left.p, right.p);
    let diff = mul(left.d, right.p) + mul(left.p, right.d);
    return DifferentialPair<vector<T,N>>(primal, diff);
}


// matrix-matrix
__generic<T : __BuiltinFloatingPointType, let R : int, let N : int, let C : int>
[ForceInline]
[ForwardDerivativeOf(mul)]
DifferentialPair<matrix<T,R,C>> mul(DifferentialPair<matrix<T,R,N>> right, DifferentialPair<matrix<T,N,C>> left)
{
    let primal = mul(right.p, left.p);
    let diff = mul(right.d, left.p) + mul(right.p, left.d);
    return DifferentialPair<matrix<T,R,C>>(primal, diff);
}

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

#define VECTOR_MAP_D_BINARY(TYPE, COUNT, D_FUNC, LEFT, RIGHT)                                                                          \
    vector<TYPE, COUNT> result;                                                                                                        \
    vector<TYPE, COUNT>.Differential d_result;                                                                                         \
    for (int i = 0; i < N; ++i)                                                                                                        \
    {                                                                                                                                  \
        DifferentialPair<TYPE> dp_elem = D_FUNC(DifferentialPair<TYPE>(LEFT.p[i], __slang_noop_cast<TYPE.Differential>(LEFT.d[i])),    \
                                                DifferentialPair<TYPE>(RIGHT.p[i], __slang_noop_cast<TYPE.Differential>(RIGHT.d[i]))); \
        result[i] = dp_elem.p;                                                                                                         \
        d_result[i] = __slang_noop_cast<TYPE>(dp_elem.d);                                                                              \
    }                                                                                                                                  \
    return DifferentialPair<vector<TYPE, COUNT>>(result, d_result)

#define VECTOR_MAP_BWD_D_UNARY(TYPE, COUNT, D_FUNC, VALUE, D_OUT)            \
    vector<TYPE, COUNT>.Differential d_result;                               \
    for (int i = 0; i < N; ++i)                                              \
    {                                                                        \
        DifferentialPair<TYPE> dp_elem = diffPair(VALUE.p[i], TYPE.dzero()); \
        D_FUNC(dp_elem, __slang_noop_cast<TYPE.Differential>(D_OUT[i]));     \
        d_result[i] = __slang_noop_cast<TYPE>(dp_elem.d);                    \
    }                                                                        \
    VALUE = diffPair(VALUE.p, d_result)

#define VECTOR_MAP_BWD_D_BINARY(TYPE, COUNT, D_FUNC, LEFT, RIGHT, D_OUT)           \
    vector<TYPE, COUNT>.Differential left_d_result, right_d_result;                \
    for (int i = 0; i < N; ++i)                                                    \
    {                                                                              \
        DifferentialPair<TYPE> left_dp = diffPair(LEFT.p[i], TYPE.dzero());        \
        DifferentialPair<TYPE> right_dp = diffPair(RIGHT.p[i], TYPE.dzero());      \
        D_FUNC(left_dp, right_dp, __slang_noop_cast<TYPE.Differential>(D_OUT[i])); \
        left_d_result[i] = __slang_noop_cast<TYPE>(left_dp.d);                     \
        right_d_result[i] = __slang_noop_cast<TYPE>(right_dp.d);                   \
    }                                                                              \
    LEFT = diffPair(LEFT.p, left_d_result);                                        \
    RIGHT = diffPair(RIGHT.p, right_d_result)

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

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(detach)]
DifferentialPair<T> __d_detach(inout DifferentialPair<T> dpx, T.Differential dOut)
{
    dpx = diffPair(dpx.p, T.dzero());
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(detach)]
void __d_detach_vector(inout DifferentialPair<vector<T, N>> dpx, vector<T, N>.Differential dOut)
{
    dpx = diffPair(dpx.p, vector<T, N>.dzero());
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

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(exp)]
void __d_exp(inout DifferentialPair<T> dpx, T.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        T.dmul(exp(dpx.p), dOut));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(exp)]
void __d_exp_vector(inout DifferentialPair<vector<T, N>> dpx, vector<T, N>.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        vector<T, N>.dmul(exp(dpx.p), dOut));
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
DifferentialPair<T> __d_sin(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        sin(dpx.p),
        T.dmul(cos(dpx.p), dpx.d));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(sin)]
DifferentialPair<vector<T, N>> __d_sin_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_sin, dpx);
}

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(sin)]
void __d_sin(inout DifferentialPair<T> dpx, T.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        T.dmul(cos(dpx.p), dOut));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(sin)]
void __d_sin_vector(inout DifferentialPair<vector<T, N>> dpx, vector<T, N>.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        vector<T, N>.dmul(cos(dpx.p), dOut));
}

// Cosine

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(cos)]
DifferentialPair<T> __d_cos(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        cos(dpx.p),
        T.dmul(-sin(dpx.p), dpx.d));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(cos)]
DifferentialPair<vector<T, N>> __d_cos_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_cos, dpx);
}

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(cos)]
void __d_cos(inout DifferentialPair<T> dpx, T.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        T.dmul(-sin(dpx.p), dOut));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(cos)]
void __d_cos_vector(inout DifferentialPair<vector<T, N>> dpx, vector<T, N>.Differential dOut)
{
    dpx = diffPair(
        dpx.p,
        vector<T, N>.dmul(-sin(dpx.p), dOut));
}

// Base-e logarithm

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(log)]
DifferentialPair<T> __d_log(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>(
        log(dpx.p),
        T.dmul(T(1.0) / dpx.p, dpx.d)
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(log)]
DifferentialPair<vector<T, N>> __d_log_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_log, dpx);
}

// Square root

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(sqrt)]
DifferentialPair<T> __d_sqrt(DifferentialPair<T> dpx)
{
    // Special case
    if (dpx.p < T(1e-6))
    {
        return DifferentialPair<T>(T(0.0), T.dzero());
    }

    T val = sqrt(dpx.p);
    return DifferentialPair<T>(
        val,
        T.dmul(T(0.5) / val, dpx.d)
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(sqrt)]
DifferentialPair<vector<T, N>> __d_sqrt_vector(DifferentialPair<vector<T, N>> dpx)
{
    VECTOR_MAP_D_UNARY(T, N, __d_sqrt, dpx);
}

// Maximum

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(max)]
DifferentialPair<T> __d_max(DifferentialPair<T> dpx, DifferentialPair<T> dpy)
{
    return DifferentialPair<T>(
        max(dpx.p, dpy.p),
        dpx.p > dpy.p ? dpx.d : dpy.d
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(max)]
DifferentialPair<vector<T, N>> __d_max_vector(DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)
{
    VECTOR_MAP_D_BINARY(T, N, __d_max, dpx, dpy);
}

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(max)]
void __d_max(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, T.Differential dOut)
{
    dpx = diffPair(dpx.p, dpx.p > dpy.p ? dOut : T.dzero());
    dpy = diffPair(dpy.p, dpy.p > dpx.p ? dOut : T.dzero());
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(max)]
DifferentialPair<vector<T, N>> __d_max_vector(inout DifferentialPair<vector<T, N>> dpx, inout DifferentialPair<vector<T, N>> dpy, vector<T, N>.Differential dOut)
{
    VECTOR_MAP_BWD_D_BINARY(T, N, __d_max, dpx, dpy, dOut);
}

// Minimum

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(min)]
DifferentialPair<T> __d_min(DifferentialPair<T> dpx, DifferentialPair<T> dpy)
{
    return DifferentialPair<T>(
        min(dpx.p, dpy.p),
        dpx.p < dpy.p ? dpx.d : dpy.d
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(min)]
DifferentialPair<vector<T, N>> __d_min_vector(DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)
{
    VECTOR_MAP_D_BINARY(T, N, __d_min, dpx, dpy);
}

// Raise to a power

__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(pow)]
DifferentialPair<T> __d_pow(DifferentialPair<T> dpx, DifferentialPair<T> dpy)
{
    // Special case
    if (dpx.p < T(1e-6))
    {
        return DifferentialPair<T>(T(0.0), T.dzero());
    }

    T val = pow(dpx.p, dpy.p);
    T.Differential d1 = T.dmul(val * log(dpx.p), dpy.d);
    T.Differential d2 = T.dmul(val * dpy.p / dpx.p, dpx.d);
    return DifferentialPair<T>(
        val,
        T.dadd(d1, d2)
    );
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(pow)]
DifferentialPair<vector<T, N>> __d_pow_vector(DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)
{
    VECTOR_MAP_D_BINARY(T, N, __d_pow, dpx, dpy);
}

// Vector dot product

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(dot)]
DifferentialPair<T> __d_dot(DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)
{
    T result = T(0);
    T.Differential d_result = T.dzero();
    for (int i = 0; i < N; ++i)
    {
        result = result + dpx.p[i] * dpy.p[i];
        d_result = T.dadd(d_result, T.dmul(dpx.p[i], __slang_noop_cast<T.Differential>(dpy.d[i])));
        d_result = T.dadd(d_result, T.dmul(dpy.p[i], __slang_noop_cast<T.Differential>(dpx.d[i])));
    }
    return DifferentialPair<T>(result, d_result);
}
