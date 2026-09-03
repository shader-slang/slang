implementing differentiable;

#define NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(METHOD, ADAPTER)              \
    [Differentiable] [ForceInline]                                              \
    This METHOD() { return ADAPTER(this); }

#define NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(METHOD, ADAPTER)             \
    [Differentiable] [ForceInline]                                              \
    This METHOD(This other) { return ADAPTER(this, other); }

#define NUMERICS_DIFFERENTIABLE_FRACTIONAL_REQUIREMENTS                         \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(adding, __differentiableNumericsAdd) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(subtracting, __differentiableNumericsSub) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(multipliedBy, __differentiableNumericsMul) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(dividedBy, __differentiableNumericsDiv) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(negated, __differentiableNumericsNeg) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        absoluteValue, __differentiableNumericsAbs)                             \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        reciprocal, __differentiableNumericsRcp)

#define NUMERICS_DIFFERENTIABLE_FLOATING_POINT_REQUIREMENTS                     \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        fractionalPart, __differentiableNumericsFractionalPart)                 \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        remainderWhenDividedBy, __differentiableNumericsRemainder)              \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        copyingSignFrom, __differentiableNumericsCopySign)

#define NUMERICS_DIFFERENTIABLE_EXPONENTIAL_REQUIREMENTS                        \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(exponential, __differentiableNumericsExp) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(exponentialWithBase2, __differentiableNumericsExp2) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        exponentialWithBase10, __differentiableNumericsRegisteredExp10)         \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(logarithm, __differentiableNumericsLog) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(logarithmWithBase2, __differentiableNumericsLog2) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        logarithmWithBase10, __differentiableNumericsLog10)                     \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        raisedToThePowerOf, __differentiableNumericsPow)

#define NUMERICS_DIFFERENTIABLE_TRIGONOMETRIC_REQUIREMENTS                      \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(sin, __differentiableNumericsSin) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(cos, __differentiableNumericsCos) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(tan, __differentiableNumericsTan) \
    [Differentiable] [ForceInline]                                              \
    void sincos(out This sineResult, out This cosineResult)                      \
    {                                                                            \
        __differentiableNumericsSinCos(this, sineResult, cosineResult);          \
    }

#define NUMERICS_DIFFERENTIABLE_INVERSE_TRIGONOMETRIC_REQUIREMENTS              \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(asin, __differentiableNumericsAsin) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(acos, __differentiableNumericsAcos) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(atan, __differentiableNumericsAtan) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        atan2, __differentiableNumericsAtan2)

#define NUMERICS_DIFFERENTIABLE_HYPERBOLIC_REQUIREMENTS                         \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(sinh, __differentiableNumericsSinh) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(cosh, __differentiableNumericsCosh) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(tanh, __differentiableNumericsTanh) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        asinh, __differentiableNumericsAsinh)                                   \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        acosh, __differentiableNumericsAcosh)                                   \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        atanh, __differentiableNumericsAtanh)

#define NUMERICS_DIFFERENTIABLE_ROOT_REQUIREMENTS                               \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(squareRoot, __differentiableNumericsSqrt) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(                                  \
        reciprocalSquareRoot, __differentiableNumericsRsqrt)

#define NUMERICS_DIFFERENTIABLE_ORDERING_REQUIREMENTS                           \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        minimum, __differentiableNumericsMinimum)                               \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(                                 \
        maximum, __differentiableNumericsMaximum)

#define NUMERICS_DIFFERENTIABLE_SCALAR_CONFORMANCES(TYPE)                       \
    public extension TYPE : IScalarDifferentiableFloatingPoint                  \
    {                                                                            \
        [Differentiable] [ForceInline]                                           \
        static This fromScalar(Scalar value) { return value; }                   \
                                                                                 \
        [Differentiable] [ForceInline]                                           \
        __init<TSource : __BuiltinFloatingPointType>(TSource value)              \
        {                                                                        \
            this = __differentiableNumericsConstructFromFloatingPoint<TYPE>(value); \
        }                                                                        \
        NUMERICS_DIFFERENTIABLE_FRACTIONAL_REQUIREMENTS                         \
        NUMERICS_DIFFERENTIABLE_FLOATING_POINT_REQUIREMENTS                     \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableExponentialFunctions                 \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_EXPONENTIAL_REQUIREMENTS                        \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableTrigonometricFunctions               \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_TRIGONOMETRIC_REQUIREMENTS                      \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableInverseTrigonometricFunctions        \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_INVERSE_TRIGONOMETRIC_REQUIREMENTS              \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableHyperbolicFunctions                  \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_HYPERBOLIC_REQUIREMENTS                         \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableRootFunctions                        \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_ROOT_REQUIREMENTS                               \
    }                                                                            \
                                                                                 \
    public extension TYPE : IDifferentiableRealOrderingFunctions                \
    {                                                                            \
        NUMERICS_DIFFERENTIABLE_ORDERING_REQUIREMENTS                           \
    }

NUMERICS_DIFFERENTIABLE_SCALAR_CONFORMANCES(half)
NUMERICS_DIFFERENTIABLE_SCALAR_CONFORMANCES(float)
NUMERICS_DIFFERENTIABLE_SCALAR_CONFORMANCES(double)

#undef NUMERICS_DIFFERENTIABLE_SCALAR_CONFORMANCES

public extension<
    T : __BuiltinFloatingPointType & IScalarDifferentiableFloatingPoint,
    let N : int>
    vector<T, N> : IDifferentiableFloatingPoint
{
    [Differentiable] [ForceInline]
    static This fromScalar(Scalar value)
    {
        return __differentiableNumericsConstructVectorFromFloatingPoint<T, T, N>(value);
    }

    [Differentiable] [ForceInline]
    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this = __differentiableNumericsConstructVectorFromFloatingPoint<T, TSource, N>(value);
    }

    NUMERICS_DIFFERENTIABLE_FRACTIONAL_REQUIREMENTS
    NUMERICS_DIFFERENTIABLE_FLOATING_POINT_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableExponentialFunctions,
    let N : int>
    vector<T, N> : IDifferentiableExponentialFunctions
{
    NUMERICS_DIFFERENTIABLE_EXPONENTIAL_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableTrigonometricFunctions,
    let N : int>
    vector<T, N> : IDifferentiableTrigonometricFunctions
{
    NUMERICS_DIFFERENTIABLE_TRIGONOMETRIC_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableInverseTrigonometricFunctions,
    let N : int>
    vector<T, N> : IDifferentiableInverseTrigonometricFunctions
{
    NUMERICS_DIFFERENTIABLE_INVERSE_TRIGONOMETRIC_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableHyperbolicFunctions,
    let N : int>
    vector<T, N> : IDifferentiableHyperbolicFunctions
{
    NUMERICS_DIFFERENTIABLE_HYPERBOLIC_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableRootFunctions,
    let N : int>
    vector<T, N> : IDifferentiableRootFunctions
{
    NUMERICS_DIFFERENTIABLE_ROOT_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IScalarDifferentiableReal,
    let N : int>
    vector<T, N> : IDifferentiableRealOrderingFunctions
{
    NUMERICS_DIFFERENTIABLE_ORDERING_REQUIREMENTS
}

#define NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(METHOD, ADAPTER)       \
    [Differentiable] [ForceInline]                                              \
    This METHOD()                                                               \
    {                                                                            \
        This result;                                                             \
        [ForceUnroll] for (int row = 0; row < R; ++row)                         \
            [ForceUnroll] for (int column = 0; column < C; ++column)            \
                result[row][column] = ADAPTER(this[row][column]);                \
        return result;                                                           \
    }

#define NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(METHOD, ADAPTER)      \
    [Differentiable] [ForceInline]                                              \
    This METHOD(This other)                                                     \
    {                                                                            \
        This result;                                                             \
        [ForceUnroll] for (int row = 0; row < R; ++row)                         \
            [ForceUnroll] for (int column = 0; column < C; ++column)            \
                result[row][column] =                                           \
                    ADAPTER(this[row][column], other[row][column]);              \
        return result;                                                           \
    }

#define NUMERICS_DIFFERENTIABLE_MATRIX_FRACTIONAL_REQUIREMENTS                  \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(adding, __differentiableNumericsAdd) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(subtracting, __differentiableNumericsSub) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(multipliedBy, __differentiableNumericsMul) \
    NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT(dividedBy, __differentiableNumericsDiv) \
    NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT(negated, __differentiableNumericsNeg) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(                           \
        absoluteValue, __differentiableNumericsAbs)                             \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(                           \
        reciprocal, __differentiableNumericsRcp)

#define NUMERICS_DIFFERENTIABLE_MATRIX_FLOATING_POINT_REQUIREMENTS              \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(                           \
        fractionalPart, __differentiableNumericsFractionalPart)                 \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(                          \
        remainderWhenDividedBy, __differentiableNumericsRemainder)              \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(                          \
        copyingSignFrom, __differentiableNumericsCopySign)

#define NUMERICS_DIFFERENTIABLE_MATRIX_EXPONENTIAL_REQUIREMENTS                 \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(exponential, __differentiableNumericsExp) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(exponentialWithBase2, __differentiableNumericsExp2) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(                           \
        exponentialWithBase10, __differentiableNumericsRegisteredExp10)         \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(logarithm, __differentiableNumericsLog) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(logarithmWithBase2, __differentiableNumericsLog2) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(logarithmWithBase10, __differentiableNumericsLog10) \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(raisedToThePowerOf, __differentiableNumericsPow)

#define NUMERICS_DIFFERENTIABLE_MATRIX_TRIGONOMETRIC_REQUIREMENTS               \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(sin, __differentiableNumericsSin) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(cos, __differentiableNumericsCos) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(tan, __differentiableNumericsTan) \
    [Differentiable] [ForceInline]                                              \
    void sincos(out This sineResult, out This cosineResult)                      \
    {                                                                            \
        [ForceUnroll] for (int row = 0; row < R; ++row)                         \
            [ForceUnroll] for (int column = 0; column < C; ++column)            \
                __differentiableNumericsSinCos(                                 \
                    this[row][column],                                           \
                    sineResult[row][column],                                     \
                    cosineResult[row][column]);                                  \
    }

#define NUMERICS_DIFFERENTIABLE_MATRIX_INVERSE_REQUIREMENTS                     \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(asin, __differentiableNumericsAsin) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(acos, __differentiableNumericsAcos) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(atan, __differentiableNumericsAtan) \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(atan2, __differentiableNumericsAtan2)

#define NUMERICS_DIFFERENTIABLE_MATRIX_HYPERBOLIC_REQUIREMENTS                  \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(sinh, __differentiableNumericsSinh) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(cosh, __differentiableNumericsCosh) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(tanh, __differentiableNumericsTanh) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(asinh, __differentiableNumericsAsinh) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(acosh, __differentiableNumericsAcosh) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(atanh, __differentiableNumericsAtanh)

#define NUMERICS_DIFFERENTIABLE_MATRIX_ROOT_REQUIREMENTS                        \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(squareRoot, __differentiableNumericsSqrt) \
    NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT(reciprocalSquareRoot, __differentiableNumericsRsqrt)

#define NUMERICS_DIFFERENTIABLE_MATRIX_ORDERING_REQUIREMENTS                    \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(                          \
        minimum, __differentiableNumericsMinimum)                               \
    NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT(                          \
        maximum, __differentiableNumericsMaximum)

public extension<
    T : __BuiltinFloatingPointType & IScalarDifferentiableFloatingPoint,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableFloatingPoint
{
    [Differentiable] [ForceInline]
    static This fromScalar(Scalar value)
    {
        return __differentiableNumericsConstructMatrixFromFloatingPoint<T, T, R, C, L>(value);
    }

    [Differentiable] [ForceInline]
    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this = __differentiableNumericsConstructMatrixFromFloatingPoint<
            T, TSource, R, C, L>(value);
    }

    NUMERICS_DIFFERENTIABLE_MATRIX_FRACTIONAL_REQUIREMENTS
    NUMERICS_DIFFERENTIABLE_MATRIX_FLOATING_POINT_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableExponentialFunctions,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableExponentialFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_EXPONENTIAL_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableTrigonometricFunctions,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableTrigonometricFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_TRIGONOMETRIC_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableInverseTrigonometricFunctions,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableInverseTrigonometricFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_INVERSE_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableHyperbolicFunctions,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableHyperbolicFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_HYPERBOLIC_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IDifferentiableRootFunctions,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableRootFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_ROOT_REQUIREMENTS
}

public extension<
    T : __BuiltinFloatingPointType & IScalarDifferentiableReal,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IDifferentiableRealOrderingFunctions
{
    NUMERICS_DIFFERENTIABLE_MATRIX_ORDERING_REQUIREMENTS
}

#undef NUMERICS_DIFFERENTIABLE_MATRIX_ORDERING_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_ROOT_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_HYPERBOLIC_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_INVERSE_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_TRIGONOMETRIC_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_EXPONENTIAL_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_FLOATING_POINT_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_FRACTIONAL_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_MATRIX_BINARY_REQUIREMENT
#undef NUMERICS_DIFFERENTIABLE_MATRIX_UNARY_REQUIREMENT
#undef NUMERICS_DIFFERENTIABLE_ORDERING_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_ROOT_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_HYPERBOLIC_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_INVERSE_TRIGONOMETRIC_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_TRIGONOMETRIC_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_EXPONENTIAL_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_FLOATING_POINT_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_FRACTIONAL_REQUIREMENTS
#undef NUMERICS_DIFFERENTIABLE_BINARY_REQUIREMENT
#undef NUMERICS_DIFFERENTIABLE_UNARY_REQUIREMENT
