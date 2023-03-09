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
attribute_syntax [PrimalSubstitute(function)] : PrimalSubstituteAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDifferentiable] : BackwardDifferentiableAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDerivativeOf(function)] : ForwardDerivativeOfAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDerivativeOf(function)] : BackwardDerivativeOfAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [PrimalSubstituteOf(function)] : PrimalSubstituteOfAttribute;

__attributeTarget(DeclBase)
attribute_syntax [DerivativeMember(memberName)] : DerivativeMemberAttribute;

// Exclude "this" parameter from differentiation.
__attributeTarget(FunctionDeclBase)
attribute_syntax [NoDiffThis] : NoDiffThisAttribute;


/// Pair type that serves to wrap the primal and
/// differential types of an arbitrary type T.

__generic<T : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairUserCodeType))
struct DifferentialPair : IDifferentiable
{
    typedef DifferentialPair<T.Differential> Differential;
    typedef T.Differential DifferentialElementType;

    __intrinsic_op($(kIROp_MakeDifferentialPairUserCode))
    __init(T _primal, T.Differential _differential);

    property p : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimalUserCode))
        get;
    }

    property v : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimalUserCode))
        get;
    }

    property d : T.Differential
    {
        __intrinsic_op($(kIROp_DifferentialPairGetDifferentialUserCode))
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
__intrinsic_op($(kIROp_MakeDifferentialPairUserCode))
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

// Matrix transpose
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[ForceInline]
[ForwardDerivativeOf(transpose)]
DifferentialPair<matrix<T, M, N>> __d_transpose(DifferentialPair<matrix<T, N, M>> m)
{
    return DifferentialPair<matrix<T, M, N>>(transpose(m.p), transpose(m.d));
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[ForceInline]
[BackwardDerivativeOf(transpose)]
void __d_transpose(inout DifferentialPair<matrix<T, N, M>> m, matrix<T, M, N>.Differential dOut)
{
    m = diffPair(m.p, transpose(dOut));
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

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[BackwardDerivativeOf(mul)]
void __d_mul(inout DifferentialPair<vector<T, N>> left, inout DifferentialPair<matrix<T, N, M>> right, vector<T, M>.Differential dOut)
{
    vector<T, N>.Differential left_d_result;
    matrix<T, N, M>.Differential right_d_result;
    for (int i = 0; i < N; ++i)
    {
        T sum = T(0);
        for (int j = 0; j < M; ++j)
        {
            sum += right.p[i][j] * dOut[j];
            right_d_result[i][j] = left.p[i] * dOut[j];
        }
        left_d_result[i] = sum;
    }
    left = diffPair(left.p, left_d_result);
    right = diffPair(right.p, right_d_result);
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

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[BackwardDerivativeOf(mul)]
void __d_mul(inout DifferentialPair<matrix<T, N, M>> left, inout DifferentialPair<vector<T, M>> right, vector<T, N>.Differential dOut)
{
    matrix<T, N, M>.Differential left_d_result;
    vector<T, M>.Differential right_d_result;
    for (int j = 0; j < M; ++j)
    {
        T sum = T(0);
        for (int i = 0; i < N; ++i)
        {
            sum += left.p[i][j] * dOut[i];
            left_d_result[i][j] = right.p[j] * dOut[i];
        }
        right_d_result[j] = sum;
    }
    left = diffPair(left.p, left_d_result);
    right = diffPair(right.p, right_d_result);
}

// matrix-matrix
__generic<T : __BuiltinFloatingPointType, let R : int, let N : int, let C : int>
[ForceInline]
[ForwardDerivativeOf(mul)]
DifferentialPair<matrix<T,R,C>> mul(DifferentialPair<matrix<T,R,N>> left, DifferentialPair<matrix<T,N,C>> right)
{
    let primal = mul(left.p, right.p);
    let diff = mul(left.d, right.p) + mul(left.p, right.d);
    return DifferentialPair<matrix<T,R,C>>(primal, diff);
}

__generic<T : __BuiltinFloatingPointType, let R : int, let N : int, let C : int>
[BackwardDerivativeOf(mul)]
void mul(inout DifferentialPair<matrix<T, R, N>> left, inout DifferentialPair<matrix<T, N, C>> right, matrix<T, R, C>.Differential dOut)
{
    matrix<T, R, N>.Differential left_d_result;
    for (int r = 0; r < R; ++r)
        for (int n = 0; n < N; ++n)
            left_d_result[r][n] = T(0.0);
    
    matrix<T, N, C>.Differential right_d_result;
    for (int n = 0; n < N; ++n)
        for (int c = 0; c < C; ++c)
            right_d_result[n][c] = T(0.0);
    
    for (int r = 0; r < R; ++r)
    {
        for (int c = 0; c < C; ++c)
        {
            for (int n = 0; n < N; ++n)
            {
                left_d_result[r][n] += right.p[n][c] * dOut[r][c];
                right_d_result[n][c] += left.p[r][n] * dOut[r][c];
            }
        }
    }
    left = diffPair(left.p, left_d_result);
    right = diffPair(right.p, right_d_result);
}

// Vector dot product
__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(dot)]
DifferentialPair<T> __d_dot(DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)
{
    T result = T(0);
    T.Differential d_result = T.dzero();
    [ForceUnroll]
    for (int i = 0; i < N; ++i)
    {
        result = result + dpx.p[i] * dpy.p[i];
        d_result = T.dadd(d_result, T.dmul(dpx.p[i], __slang_noop_cast<T.Differential>(dpy.d[i])));
        d_result = T.dadd(d_result, T.dmul(dpy.p[i], __slang_noop_cast<T.Differential>(dpx.d[i])));
    }
    return DifferentialPair<T>(result, d_result);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(dot)]
void __d_dot(inout DifferentialPair<vector<T, N>> dpx, inout DifferentialPair<vector<T, N>> dpy, T.Differential dOut)
{
    vector<T, N>.Differential x_d_result, y_d_result;
    [ForceUnroll]
    for (int i = 0; i < N; ++i)
    {
        x_d_result[i] = dpy.p[i] * __slang_noop_cast<T>(dOut);
        y_d_result[i] = dpx.p[i] * __slang_noop_cast<T>(dOut);
    }
    dpx = diffPair(dpx.p, x_d_result);
    dpy = diffPair(dpy.p, y_d_result);
}

// Cross product
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(cross)]
DifferentialPair<vector<T, 3>> __d_cross(DifferentialPair<vector<T, 3>> a, DifferentialPair<vector<T, 3>> b)
{
    /*
    cx = ay * bz − az * by
    cy = az * bx − ax * bz
    cz = ax * by − ay * bx
    */
    T aybz = a.p.y * b.p.z;
    T azby = a.p.z * b.p.y;
    T px = aybz - azby;
    T dx = (b.p.z - azby) * a.d.y + (a.p.y - azby) * b.d.z + (aybz - b.p.y) * a.d.z + (aybz - a.p.z) * b.d.y;

    T azbx = a.p.z * b.p.x;
    T axbz = a.p.x * b.p.z;
    T py = azbx - axbz;
    T dy = (b.p.x - axbz) * a.d.z + (a.p.z - axbz) * b.d.x + (azbx - b.p.z) * a.d.x + (azbx - a.p.x) * b.d.z;

    T axby = a.p.x * b.p.y;
    T aybx = a.p.y * b.p.x;
    T pz = axby - aybx;
    T dz = (b.p.y - aybx) * a.d.x + (a.p.x - aybx) * b.d.y + (axby - b.p.x) * a.d.y + (axby - a.p.y) * b.d.x;
    
    return DifferentialPair<vector<T, 3>>(vector<T, 3>(px, py, pz), vector<T, 3>.Differential(dx, dy, dz));
}

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(cross)]
void __d_cross(inout DifferentialPair<vector<T, 3>> a, inout DifferentialPair<vector<T, 3>> b, vector<T, 3>.Differential dOut)
{
    /*
    cx = ay * bz − az * by
    cy = az * bx − ax * bz
    cz = ax * by − ay * bx
    */
    T dax = (-b.p.z * dOut.y) + (b.p.y * dOut.z);
    T day = (b.p.z * dOut.x) + (-b.p.x * dOut.z);
    T daz = (-b.p.y * dOut.x) + (b.p.x * dOut.y);

    T dbx = (a.p.z * dOut.y) + (-a.p.y * dOut.z);
    T dby = (-a.p.z * dOut.x) + (a.p.x * dOut.z);
    T dbz = (a.p.y * dOut.x) + (-a.p.x * dOut.y);

    a = diffPair(a.p, vector<T, 3>.Differential(dax, day, daz));
    b = diffPair(b.p, vector<T, 3>.Differential(dbx, dby, dbz));
}

#define VECTOR_MATRIX_BINARY_DIFF_IMPL(NAME)                                                 \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<vector<T, N>> __d_##NAME##_vector(                                      \
        DifferentialPair<vector<T, N>> dpx, DifferentialPair<vector<T, N>> dpy)              \
    {                                                                                        \
        vector<T, N> result;                                                                 \
        vector<T, N>.Differential d_result;                                                  \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
                DifferentialPair<T>(dpx.p[i], __slang_noop_cast<T.Differential>(dpx.d[i])),  \
                DifferentialPair<T>(dpy.p[i], __slang_noop_cast<T.Differential>(dpy.d[i]))); \
            result[i] = dp_elem.p;                                                           \
            d_result[i] = __slang_noop_cast<T>(dp_elem.d);                                   \
        }                                                                                    \
        return DifferentialPair<vector<T, N>>(result, d_result);                             \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<matrix<T, M, N>> __d_##NAME##_matrix(                                   \
        DifferentialPair<matrix<T, M, N>> dpx, DifferentialPair<matrix<T, M, N>> dpy)        \
    {                                                                                        \
        matrix<T, M, N> result;                                                              \
        matrix<T, M, N>.Differential d_result;                                               \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
                DifferentialPair<T>(dpx.p[i][j], __slang_noop_cast<T.Differential>(dpx.d[i][j])),  \
                DifferentialPair<T>(dpy.p[i][j], __slang_noop_cast<T.Differential>(dpy.d[i][j]))); \
            result[i][j] = dp_elem.p;                                                        \
            d_result[i][j] = __slang_noop_cast<T>(dp_elem.d);                                \
        }                                                                                    \
        return DifferentialPair<matrix<T, M, N>>(result, d_result);                          \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_vector(                                                                \
            inout DifferentialPair<vector<T, N>> dpx,                                        \
            inout DifferentialPair<vector<T, N>> dpy,                                        \
            vector<T, N>.Differential dOut)                                                  \
    {                                                                                        \
        vector<T, N>.Differential left_d_result, right_d_result;                             \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> left_dp = diffPair(dpx.p[i], T.dzero());                     \
            DifferentialPair<T> right_dp = diffPair(dpy.p[i], T.dzero());                    \
            __d_##NAME(left_dp, right_dp, __slang_noop_cast<T.Differential>(dOut[i]));       \
            left_d_result[i] = __slang_noop_cast<T>(left_dp.d);                              \
            right_d_result[i] = __slang_noop_cast<T>(right_dp.d);                            \
        }                                                                                    \
        dpx = diffPair(dpx.p, left_d_result);                                                \
        dpy = diffPair(dpy.p, right_d_result);                                               \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_matrix(                                                                \
            inout DifferentialPair<matrix<T, M, N>> dpx,                                     \
            inout DifferentialPair<matrix<T, M, N>> dpy,                                     \
            matrix<T, M, N>.Differential dOut)                                               \
    {                                                                                        \
        matrix<T, M, N>.Differential left_d_result, right_d_result;                          \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> left_dp = diffPair(dpx.p[i][j], T.dzero());                  \
            DifferentialPair<T> right_dp = diffPair(dpy.p[i][j], T.dzero());                 \
            __d_##NAME(left_dp, right_dp, __slang_noop_cast<T.Differential>(dOut[i][j]));    \
            left_d_result[i][j] = __slang_noop_cast<T>(left_dp.d);                           \
            right_d_result[i][j] = __slang_noop_cast<T>(right_dp.d);                         \
        }                                                                                    \
        dpx = diffPair(dpx.p, left_d_result);                                                \
        dpy = diffPair(dpy.p, right_d_result);                                               \
    }

#define VECTOR_MATRIX_TERNARY_DIFF_IMPL(NAME)                                                \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<vector<T, N>> __d_##NAME##_vector(                                      \
        DifferentialPair<vector<T, N>> dpx,                                                  \
        DifferentialPair<vector<T, N>> dpy,                                                  \
        DifferentialPair<vector<T, N>> dpz)                                                  \
    {                                                                                        \
        vector<T, N> result;                                                                 \
        vector<T, N>.Differential d_result;                                                  \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
                DifferentialPair<T>(dpx.p[i], __slang_noop_cast<T.Differential>(dpx.d[i])),  \
                DifferentialPair<T>(dpy.p[i], __slang_noop_cast<T.Differential>(dpy.d[i])),  \
                DifferentialPair<T>(dpz.p[i], __slang_noop_cast<T.Differential>(dpz.d[i]))); \
            result[i] = dp_elem.p;                                                           \
            d_result[i] = __slang_noop_cast<T>(dp_elem.d);                                   \
        }                                                                                    \
        return DifferentialPair<vector<T, N>>(result, d_result);                             \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<matrix<T, M, N>> __d_##NAME##_matrix(                                  \
        DifferentialPair<matrix<T, M, N>> dpx,                                               \
        DifferentialPair<matrix<T, M, N>> dpy,                                               \
        DifferentialPair<matrix<T, M, N>> dpz)                                               \
    {                                                                                        \
        matrix<T, M, N> result;                                                              \
        matrix<T, M, N>.Differential d_result;                                               \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
                DifferentialPair<T>(dpx.p[i][j], __slang_noop_cast<T.Differential>(dpx.d[i][j])),  \
                DifferentialPair<T>(dpy.p[i][j], __slang_noop_cast<T.Differential>(dpy.d[i][j])),  \
                DifferentialPair<T>(dpz.p[i][j], __slang_noop_cast<T.Differential>(dpz.d[i][j]))); \
            result[i][j] = dp_elem.p;                                                        \
            d_result[i][j] = __slang_noop_cast<T>(dp_elem.d);                                \
        }                                                                                    \
        return DifferentialPair<matrix<T, M, N>>(result, d_result);                          \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_vector(                                                                \
            inout DifferentialPair<vector<T, N>> dpx,                                        \
            inout DifferentialPair<vector<T, N>> dpy,                                        \
            inout DifferentialPair<vector<T, N>> dpz,                                        \
            vector<T, N>.Differential dOut)                                                  \
    {                                                                                        \
        vector<T, N>.Differential left_d_result, middle_d_result, right_d_result;            \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> left_dp = diffPair(dpx.p[i], T.dzero());                     \
            DifferentialPair<T> middle_dp = diffPair(dpy.p[i], T.dzero());                   \
            DifferentialPair<T> right_dp = diffPair(dpz.p[i], T.dzero());                    \
            __d_##NAME(left_dp, middle_dp, right_dp,                                         \
                __slang_noop_cast<T.Differential>(dOut[i]));                                 \
            left_d_result[i] = __slang_noop_cast<T>(left_dp.d);                              \
            middle_d_result[i] = __slang_noop_cast<T>(middle_dp.d);                          \
            right_d_result[i] = __slang_noop_cast<T>(right_dp.d);                            \
        }                                                                                    \
        dpx = diffPair(dpx.p, left_d_result);                                                \
        dpy = diffPair(dpy.p, middle_d_result);                                              \
        dpz = diffPair(dpz.p, right_d_result);                                               \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_matrix(                                                                \
            inout DifferentialPair<matrix<T, M, N>> dpx,                                     \
            inout DifferentialPair<matrix<T, M, N>> dpy,                                     \
            inout DifferentialPair<matrix<T, M, N>> dpz,                                     \
            matrix<T, M, N>.Differential dOut)                                               \
    {                                                                                        \
        matrix<T, M, N>.Differential left_d_result, middle_d_result, right_d_result;         \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> left_dp = diffPair(dpx.p[i][j], T.dzero());                  \
            DifferentialPair<T> middle_dp = diffPair(dpy.p[i][j], T.dzero());                \
            DifferentialPair<T> right_dp = diffPair(dpz.p[i][j], T.dzero());                 \
            __d_##NAME(left_dp, middle_dp, right_dp,                                         \
                __slang_noop_cast<T.Differential>(dOut[i][j]));                              \
            left_d_result[i][j] = __slang_noop_cast<T>(left_dp.d);                           \
            middle_d_result[i][j] = __slang_noop_cast<T>(middle_dp.d);                       \
            right_d_result[i][j] = __slang_noop_cast<T>(right_dp.d);                         \
        }                                                                                    \
        dpx = diffPair(dpx.p, left_d_result);                                                \
        dpy = diffPair(dpy.p, middle_d_result);                                              \
        dpz = diffPair(dpz.p, right_d_result);                                               \
    }

#define UNARY_DERIVATIVE_IMPL(NAME, FWD_DIFF_FUNC, BWD_DIFF_FUNC)                            \
    __generic<T : __BuiltinFloatingPointType>                                                \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<T> __d_##NAME(DifferentialPair<T> dpx)                                  \
    {                                                                                        \
        return DifferentialPair<T>(NAME(dpx.p), FWD_DIFF_FUNC);                              \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<vector<T, N>> __d_##NAME##_vector(DifferentialPair<vector<T, N>> dpx)   \
    {                                                                                        \
        vector<T, N> result;                                                                 \
        vector<T, N>.Differential d_result;                                                  \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
                DifferentialPair<T>(dpx.p[i], __slang_noop_cast<T.Differential>(dpx.d[i]))); \
            result[i] = dp_elem.p;                                                           \
            d_result[i] = __slang_noop_cast<T>(dp_elem.d);                                   \
        }                                                                                    \
        return DifferentialPair<vector<T, N>>(result, d_result);                             \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [ForwardDerivativeOf(NAME)]                                                              \
    DifferentialPair<matrix<T, M, N>> __d_##NAME##_m(DifferentialPair<matrix<T, M, N>> dpx)  \
    {                                                                                        \
        matrix<T, M, N> result;                                                              \
        matrix<T, M, N>.Differential d_result;                                               \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = __d_##NAME(                                        \
               DifferentialPair<T>(dpx.p[i][j],                                              \
                                   __slang_noop_cast<T.Differential>(dpx.d[i][j])));         \
            result[i][j] = dp_elem.p;                                                        \
            d_result[i][j] = __slang_noop_cast<T>(dp_elem.d);                                \
        }                                                                                    \
        return DifferentialPair<matrix<T, M, N>>(result, d_result);                          \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType>                                                \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME(inout DifferentialPair<T> dpx, T.Differential dOut)                      \
    {                                                                                        \
        dpx = diffPair(dpx.p, BWD_DIFF_FUNC);                                                \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let N : int>                                   \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_vector(                                                                \
            inout DifferentialPair<vector<T, N>> dpx, vector<T, N>.Differential dOut)        \
    {                                                                                        \
        vector<T, N>.Differential d_result;                                                  \
        [ForceUnroll] for (int i = 0; i < N; ++i)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = diffPair(dpx.p[i], T.dzero());                     \
            __d_##NAME(dp_elem, __slang_noop_cast<T.Differential>(dOut[i]));                 \
            d_result[i] = __slang_noop_cast<T>(dp_elem.d);                                   \
        }                                                                                    \
        dpx = diffPair(dpx.p, d_result);                                                     \
    }                                                                                        \
    __generic<T : __BuiltinFloatingPointType, let M : int, let N : int>                      \
    [BackwardDerivativeOf(NAME)]                                                             \
    void __d_##NAME##_matrix(                                                                \
            inout DifferentialPair<matrix<T, M, N>> dpx, matrix<T, M, N>.Differential dOut)  \
    {                                                                                        \
        matrix<T, M, N>.Differential d_result;                                               \
        [ForceUnroll] for (int i = 0; i < M; ++i)                                            \
        [ForceUnroll] for (int j = 0; j < N; ++j)                                            \
        {                                                                                    \
            DifferentialPair<T> dp_elem = diffPair(dpx.p[i][j], T.dzero());                  \
            __d_##NAME(dp_elem, __slang_noop_cast<T.Differential>(dOut[i][j]));              \
            d_result[i][j] = __slang_noop_cast<T>(dp_elem.d);                                \
        }                                                                                    \
        dpx = diffPair(dpx.p, d_result);                                                     \
    }
#define SIMPLE_UNARY_DERIVATIVE_IMPL(NAME, DIFF_FUNC) UNARY_DERIVATIVE_IMPL(NAME, T.dmul(DIFF_FUNC, dpx.d), T.dmul(DIFF_FUNC, dOut))

// Detach and set derivatives to zero
__generic<T : IDifferentiable>
__intrinsic_op($(kIROp_DetachDerivative))
T detach(T x);

#define SLANG_SQR(x) ((x)*(x))

// Absolute value
UNARY_DERIVATIVE_IMPL(abs, (dpx.p > T(0.0) ? dpx.d : T.dmul(T(-1.0), dpx.d)), (T.dmul(__slang_noop_cast<T>(sign(dpx.p)), dOut)))
// Saturate
UNARY_DERIVATIVE_IMPL(saturate, (dpx.p < T(0.0) || dpx.p > T(1.0) ? T.dzero() : dpx.d), (dpx.p < T(0.0) || dpx.p > T(1.0) ? T.dzero() : dOut))
// frac
UNARY_DERIVATIVE_IMPL(frac, dpx.d, dOut)
// raidans, degrees
SIMPLE_UNARY_DERIVATIVE_IMPL(radians, T(0.01745329251994329576923690768489))
SIMPLE_UNARY_DERIVATIVE_IMPL(degrees, T(57.295779513082320876798154814105))
// Exponent
SIMPLE_UNARY_DERIVATIVE_IMPL(exp, exp(dpx.p))
SIMPLE_UNARY_DERIVATIVE_IMPL(exp2, exp2(dpx.p)* T(50.69314718055994530941723212145818))
// sin, sinh
SIMPLE_UNARY_DERIVATIVE_IMPL(sin, cos(dpx.p))
SIMPLE_UNARY_DERIVATIVE_IMPL(sinh, cosh(dpx.p))
// cos, cosh
SIMPLE_UNARY_DERIVATIVE_IMPL(cos, -sin(dpx.p))
SIMPLE_UNARY_DERIVATIVE_IMPL(cosh, sinh(dpx.p))
// tan, tanh
SIMPLE_UNARY_DERIVATIVE_IMPL(tan, T(1.0) / (cos(dpx.p) * cos(dpx.p)))
SIMPLE_UNARY_DERIVATIVE_IMPL(tanh, T(1.0) / (cosh(dpx.p) * cosh(dpx.p)))
// Logarithm
SIMPLE_UNARY_DERIVATIVE_IMPL(log, T(1.0) / dpx.p)
SIMPLE_UNARY_DERIVATIVE_IMPL(log10, T(1.0) / (dpx.p * T(52.3025850929940456840179914546844)))
SIMPLE_UNARY_DERIVATIVE_IMPL(log2, T(1.0) / (dpx.p * T(50.69314718055994530941723212145818)))
// Square root
SIMPLE_UNARY_DERIVATIVE_IMPL(sqrt, (dpx.p < T(1e-7) ? T(0.0) : T(0.5) / sqrt(dpx.p)))
// Reciprocal
SIMPLE_UNARY_DERIVATIVE_IMPL(rcp, (dpx.p < T(1e-7) ? T(0.0) : T(-1.0) / (dpx.p * dpx.p)))
// rsqrt
SIMPLE_UNARY_DERIVATIVE_IMPL(rsqrt, T(-0.5) / (dpx.p * sqrt(dpx.p)))
// Arc-sin
SIMPLE_UNARY_DERIVATIVE_IMPL(asin, T(1.0) / sqrt(T(1.0) - dpx.p * dpx.p))
// Arc-cos
SIMPLE_UNARY_DERIVATIVE_IMPL(acos, T(-1.0) / sqrt(T(1.0) - dpx.p * dpx.p))
// Arc-tan
SIMPLE_UNARY_DERIVATIVE_IMPL(atan, T(1.0) / (T(1.0) + dpx.p * dpx.p))

// Atan2
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(atan2)]
DifferentialPair<T> __d_atan2(DifferentialPair<T> dpy, DifferentialPair<T> dpx)
{
    T.Differential dx = T.dmul(-dpy.p / (dpx.p * dpx.p + dpy.p * dpy.p), dpx.d);
    T.Differential dy = T.dmul(-dpx.p / (dpx.p * dpx.p + dpy.p * dpy.p), dpy.d);
    return DifferentialPair<T>(
        atan2(dpy.p, dpx.p),
        T.dadd(dx, dy));
}

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(atan2)]
void __d_atan2(inout DifferentialPair<T> dpy, inout DifferentialPair<T> dpx, T.Differential dOut)
{
    dpx = diffPair(dpx.p, T.dmul(-dpy.p / (dpx.p * dpx.p + dpy.p * dpy.p), dpx.d));
    dpy = diffPair(dpy.p, T.dmul(-dpx.p / (dpx.p * dpx.p + dpy.p * dpy.p), dpy.d));
}

VECTOR_MATRIX_BINARY_DIFF_IMPL(atan2)

// fmod
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(fmod)]
DifferentialPair<T> __d_fmod(DifferentialPair<T> x, DifferentialPair<T> y)
{
    return DifferentialPair<T>(fmod(x.p, y.p), x.d);
}
__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(fmod)]
void __d_fmod(inout DifferentialPair<T> x, inout DifferentialPair<T> y, T.Differential dOut)
{
    x = diffPair(x.p, dOut);
    y = diffPair(y.p);
}
VECTOR_MATRIX_BINARY_DIFF_IMPL(fmod)

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

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(pow)]
void __d_pow(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, T.Differential dOut)
{
    // Special case
    if (dpx.p < T(1e-6))
    {
        dpx = diffPair(dpx.p, T.dzero());
        dpy = diffPair(dpy.p, T.dzero());
    }
    else
    {
        T val = pow(dpx.p, dpy.p);
        dpx = diffPair(
            dpx.p,
            T.dmul(val * dpy.p / dpx.p, dOut));
        dpy = diffPair(
            dpy.p,
            T.dmul(val * log(dpx.p), dOut));
    }
}

VECTOR_MATRIX_BINARY_DIFF_IMPL(pow)

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

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(max)]
void __d_max(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, T.Differential dOut)
{
    dpx = diffPair(dpx.p, dpx.p > dpy.p ? dOut : T.dzero());
    dpy = diffPair(dpy.p, dpy.p > dpx.p ? dOut : T.dzero());
}

VECTOR_MATRIX_BINARY_DIFF_IMPL(max)

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

__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(min)]
void __d_min(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, T.Differential dOut)
{
    dpx = diffPair(dpx.p, dpx.p < dpy.p ? dOut : T.dzero());
    dpy = diffPair(dpy.p, dpy.p < dpx.p ? dOut : T.dzero());
}

VECTOR_MATRIX_BINARY_DIFF_IMPL(min)

// Lerp
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(lerp)]
DifferentialPair<T> __d_lerp(DifferentialPair<T> dpx, DifferentialPair<T> dpy, DifferentialPair<T> dps)
{
    return DifferentialPair<T>(
        lerp(dpx.p, dpy.p, dps.p),
        T.dadd(T.dadd(T.dmul((T(1.0) - dps.p), dpx.d), T.dmul(dps.p, dpy.d)), T.dmul(dpy.p - dpx.p, dps.d))
    );
}
__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(lerp)]
void __d_lerp(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, inout DifferentialPair<T> dps, T.Differential dOut)
{
    dpx = diffPair(dpx.p, T.dmul(T(1.0) - dps.p, dOut));
    dpy = diffPair(dpy.p, T.dmul(dps.p, dOut));
    dps = diffPair(dpy.p, T.dmul((dpy.p - dpx.p), dOut));
}
VECTOR_MATRIX_TERNARY_DIFF_IMPL(lerp)

//  Clamp
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(clamp)]
DifferentialPair<T> __d_clamp(DifferentialPair<T> dpx, DifferentialPair<T> dpMin, DifferentialPair<T> dpMax)
{
    return DifferentialPair<T>(
        clamp(dpx.p, dpMin.p, dpMax.p),
        dpx.p < dpMin.p ? (dpx.p > dpMax.p ? dpMax.d : dpx.d) : dpMin.d);
}
__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(clamp)]
void __d_clamp(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpMin, inout DifferentialPair<T> dpMax, T.Differential dOut)
{
    dpx = diffPair(dpx.p, dpx.p > dpMin.p && dpx.p < dpMax.p ? dOut : T.dzero());
    dpMin = diffPair(dpMin.p, dpx.p <= dpMin.p ? dOut : T.dzero());
    dpMax = diffPair(dpMin.p, dpx.p >= dpMax.p ? dOut : T.dzero());
}
VECTOR_MATRIX_TERNARY_DIFF_IMPL(clamp)

// fma
[ForwardDerivativeOf(fma)]
DifferentialPair<double> __d_fma(DifferentialPair<double> dpx, DifferentialPair<double> dpy, DifferentialPair<double> dpz)
{
    return DifferentialPair<double>(
        fma(dpx.p, dpy.p, dpz.p),
        dpy.p * dpx.d + dpx.p * dpy.d + dpz.d);
}
[BackwardDerivativeOf(fma)]
void __d_fma(inout DifferentialPair<double> dpx, inout DifferentialPair<double> dpy, inout DifferentialPair<double> dpz, double dOut)
{
    dpx = diffPair(dpx.p, dpy.p * dOut);
    dpy = diffPair(dpy.p, dpx.p * dOut);
    dpz = diffPair(dpz.p, dOut);
}
__generic<let N : int>
[ForwardDerivativeOf(fma)]
DifferentialPair<vector<double, N>> __d_fma_vector(
    DifferentialPair<vector<double, N>> dpx,
    DifferentialPair<vector<double, N>> dpy,
    DifferentialPair<vector<double, N>> dpz)
{
    vector<double, N> result;
    vector<double, N>.Differential d_result;
    [ForceUnroll] for (int i = 0; i < N; ++i)
    {
        DifferentialPair<double> dp_elem = __d_fma(
            DifferentialPair<double>(dpx.p[i], dpx.d[i]),
            DifferentialPair<double>(dpy.p[i], dpy.d[i]),
            DifferentialPair<double>(dpz.p[i], dpz.d[i]));
        result[i] = dp_elem.p;
        d_result[i] = dp_elem.d;
    }
    return DifferentialPair<vector<double, N>>(result, d_result);
}
__generic<let N : int>
[BackwardDerivativeOf(fma)]
void __d_fma_vector(
        inout DifferentialPair<vector<double, N>> dpx,
        inout DifferentialPair<vector<double, N>> dpy,
        inout DifferentialPair<vector<double, N>> dpz,
        vector<double, N> dOut)
{
    vector<double, N>.Differential x_d_result, y_d_result, z_d_result;
    [ForceUnroll] for (int i = 0; i < N; ++i)
    {
        DifferentialPair<double> x_dp = diffPair(dpx.p[i], 0.0);
        DifferentialPair<double> y_dp = diffPair(dpy.p[i], 0.0);
        DifferentialPair<double> z_dp = diffPair(dpz.p[i], 0.0);
        __d_fma(x_dp, y_dp, z_dp, dOut[i]);
        x_d_result[i] = x_dp.d;
        y_d_result[i] = y_dp.d;
        z_d_result[i] = z_dp.d;
    }
    dpx = diffPair(dpx.p, x_d_result);
    dpy = diffPair(dpy.p, y_d_result);
    dpz = diffPair(dpz.p, z_d_result);
}

// mad
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(mad)]
DifferentialPair<T> __d_mad(DifferentialPair<T> dpx, DifferentialPair<T> dpy, DifferentialPair<T> dpz)
{
    return DifferentialPair<T>(
        mad(dpx.p, dpy.p, dpz.p),
        T.dadd(T.dadd(T.dmul(dpy.p, dpx.d), T.dmul(dpx.p, dpy.d)), dpz.d));
}
__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(mad)]
void __d_mad(inout DifferentialPair<T> dpx, inout DifferentialPair<T> dpy, inout DifferentialPair<T> dpz, T.Differential dOut)
{
    dpx = diffPair(dpx.p, T.dmul(dpy.p, dOut));
    dpy = diffPair(dpy.p, T.dmul(dpx.p, dOut));
    dpz = diffPair(dpz.p, dOut);
}
VECTOR_MATRIX_TERNARY_DIFF_IMPL(mad)

// Smoothstep
__generic<T : __BuiltinFloatingPointType>
[BackwardDifferentiable]
T __smoothstep_impl(T minVal, T maxVal, T x)
{
    let t = saturate((x - minVal) / (maxVal - minVal));
    return t * t * (T(3.0) - T(2.0) * t);
}
__generic<T : __BuiltinFloatingPointType>
[ForwardDerivativeOf(smoothstep)]
DifferentialPair<T> __d_smoothstep(DifferentialPair<T> minVal, DifferentialPair<T> maxVal, DifferentialPair<T> x)
{
    return __fwd_diff(__smoothstep_impl)(minVal, maxVal, x);
}
__generic<T : __BuiltinFloatingPointType>
[BackwardDerivativeOf(smoothstep)]
void __d_smoothstep(inout DifferentialPair<T> minVal, inout DifferentialPair<T> maxVal, inout DifferentialPair<T> x, T.Differential dOut)
{
    __bwd_diff(__smoothstep_impl)(minVal, maxVal, x, dOut);
}
VECTOR_MATRIX_TERNARY_DIFF_IMPL(smoothstep)

// Vector length
__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
T __length_impl(vector<T, N> x)
{
    T len = T(0.0);
    [ForceUnroll] for (int i = 0; i < N; i++)
    {
        len += x[i] * x[i];
    }
    return sqrt(len);
}

__generic<T: __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(length)]
[ForceInline]
DifferentialPair<T> __d_length(DifferentialPair<vector<T, N>> x)
{
    return __fwd_diff(__length_impl)(x);
}

__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(length)]
[ForceInline]
void __d_length(inout DifferentialPair<vector<T, N>> x, T.Differential dOut)
{
    return __bwd_diff(__length_impl)(x, dOut);
}

// Vector distance
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
T __distance_impl(vector<T, N> x, vector<T, N> y)
{
    return length(y - x);
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(distance)]
[ForceInline]
DifferentialPair<T> __d_distance(DifferentialPair<vector<T, N>> x, DifferentialPair<vector<T, N>> y)
{
    return __fwd_diff(__distance_impl)(x, y);
}

__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(distance)]
[ForceInline]
void __d_distance(inout DifferentialPair<vector<T, N>> x, inout DifferentialPair<vector<T, N>> y, T.Differential dOut)
{
    return __bwd_diff(__distance_impl)(x, y, dOut);
}

// Vector normalize
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
vector<T, N> __normalize_impl(vector<T, N> x)
{
    let r = T(1.0) / length(x);
    return x * r;
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(normalize)]
[ForceInline]
DifferentialPair<vector<T, N>> __d_normalize(DifferentialPair<vector<T, N>> x)
{
    return __fwd_diff(__normalize_impl)(x);
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(normalize)]
[ForceInline]
void __d_distance(inout DifferentialPair<vector<T, N>> x, vector<T, N>.Differential dOut)
{
    return __bwd_diff(__normalize_impl)(x, dOut);
}

// Vector reflect
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
vector<T, N> __reflect_impl(vector<T, N> i, vector<T, N> n)
{
    return  i - n * (T(2.0) * dot(i, n));
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(reflect)]
[ForceInline]
DifferentialPair<vector<T, N>> __d_reflect(DifferentialPair<vector<T, N>> i, DifferentialPair<vector<T, N>> n)
{
    return __fwd_diff(__reflect_impl)(i, n);
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(reflect)]
[ForceInline]
void __d_reflect(inout DifferentialPair<vector<T, N>> i, inout DifferentialPair<vector<T, N>> n, vector<T, N>.Differential dOut)
{
    return __bwd_diff(__reflect_impl)(i, n, dOut);
}

// Vector refract
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
vector<T, N> __refract_impl(vector<T, N> i, vector<T, N> n, T eta)
{
    let k = T(1.0) - eta * eta * (T(1.0) - dot(n, i) * dot(n, i));
    return (k < T(0.0)) ? vector<T, N>(T(0.0)) : eta * i - (eta * dot(n, i) + sqrt(max(T(0.0),k))) * n;
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(refract)]
[ForceInline]
DifferentialPair<vector<T, N>> __d_refract(DifferentialPair<vector<T, N>> i, DifferentialPair<vector<T, N>> n, DifferentialPair<T> eta)
{
    return __fwd_diff(__refract_impl)(i, n, eta);
}
__generic<T: __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(refract)]
[ForceInline]
void __d_refract(inout DifferentialPair<vector<T, N>> i, inout DifferentialPair<vector<T, N>> n, inout DifferentialPair<T> eta, vector<T, N>.Differential dOut)
{
    return __bwd_diff(__refract_impl)(i, n, eta, dOut);
}

// Sine and cosine
__generic<T : __BuiltinFloatingPointType>
[BackwardDifferentiable]
[PrimalSubstituteOf(sincos)]
void __sincos_impl(T x, out T s, out T c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
[PrimalSubstituteOf(sincos)]
void __sincos_impl(vector<T, N> x, out vector<T, N> s, out vector<T, N> c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
[BackwardDifferentiable]
[PrimalSubstituteOf(sincos)]
void __sincos_impl(matrix<T, N, M> x, out matrix<T, N, M> s, out matrix<T, N, M> c)
{
    s = sin(x);
    c = cos(x);
}


// dst (obsolete)
__generic<T : __BuiltinFloatingPointType>
[BackwardDifferentiable]
[PrimalSubstituteOf(dst)]
vector<T, 4> __dst_impl(vector<T, 4> src0, vector<T, 4> src1)
{
    vector<T, 4> dest;
    dest.x = T(1.0);
    dest.y = src0.y * src1.y;
    dest.z = src0.z;
    dest.w = src1.w; ;
    return dest;
}

// Legacy lighting function (obsolete)
[__readNone]
[BackwardDifferentiable]
[PrimalSubstituteOf(lit)]
float4 __lit_impl(float n_dot_l, float n_dot_h, float m)
{
    let ambient = 1.0f;
    let diffuse = max(n_dot_l, 0.0f);
    let specular = ((n_dot_l < 0.0f || n_dot_h < 0.0) ? 0.0 : pow(n_dot_h, m));
    return float4(ambient, diffuse, specular, 1.0f);
}
// Matrix determinant
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDifferentiable]
[__readNone]
T __determinant_impl(matrix<T,N,N> m)
{
    if (N == 1)
        return m[0][0];
    else if (N == 2)
        return m[0][0] * m[1][1] - m[0][1] * m[1][0];
    else if (N == 3)
    {
        return  m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
		      - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
			  + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
    }
    else if (N == 4)
    {
    	T s00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		T s01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		T s02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		T s03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		T s04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		T s05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

		return m[0][0] * (m[1][1] * s00 - m[1][2] * s01 + m[1][3] * s02)
			 - m[0][1] * (m[1][0] * s00 - m[1][2] * s03 + m[1][3] * s04)
			 + m[0][2] * (m[1][0] * s01 - m[1][1] * s03 + m[1][3] * s05)
			 - m[0][3] * (m[1][0] * s02 - m[1][1] * s04 + m[1][2] * s05);
    }
    return T(0.0);
}
__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(determinant)]
[ForceInline]
DifferentialPair<T> __determinant_impl(DifferentialPair<matrix<T,N,N>> m)
{
    return __fwd_diff(__determinant_impl)(m);
}
__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(determinant)]
[ForceInline]
void __d_determinant(inout DifferentialPair<matrix<T,N,N>> m, T.Differential dOut)
{
    __bwd_diff(__determinant_impl)(m, dOut);
}
