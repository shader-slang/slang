implementing numerics;

// Cooperative-vector targets do not expose every integer operation as a native aggregate
// operation, so these witnesses define the missing operations component-wise.
public extension<T : __BuiltinIntegerType & IScalarIntegerType, let N : int>
    CoopVec<T, N> : IIntegerType
{
    typealias Scalar = T;
    typealias Mask = vector<bool, N>;
    typealias ShiftAmount = CoopVec<int, N>;

    [ForceInline] [require(cooperative_vector)]
    static This fromScalar(Scalar value) { return This(value); }
    [ForceInline] [require(cooperative_vector)]
    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromInteger<T>(value));
    }
    [ForceInline] [require(cooperative_vector)]
    static This zero() { return This(Scalar.zero()); }
    [ForceInline] [require(cooperative_vector)]
    static This one() { return This(Scalar.one()); }

    [ForceInline] [require(cooperative_vector)]
    Mask isEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] == other[index];
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    Mask isLessThan(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] < other[index];
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    Mask isLessThanOrEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] <= other[index];
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    This adding(This other) { return this.add(other); }
    [ForceInline] [require(cooperative_vector)]
    This subtracting(This other) { return this.sub(other); }
    [ForceInline] [require(cooperative_vector)]
    This multipliedBy(This other) { return this.mul(other); }
    [ForceInline] [require(cooperative_vector)]
    This quotientWhenDividedBy(This divisor) { return this.div(divisor); }
    [ForceInline] [require(cooperative_vector)]
    This remainderWhenDividedBy(This divisor) { return this.mod(divisor); }

    [ForceInline] [require(cooperative_vector)]
    This shiftedLeftBy(ShiftAmount amount)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index].shl(amount[index]);
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    This shiftedRightBy(ShiftAmount amount)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index].shr(amount[index]);
        return result;
    }

#define NUMERICS_COOPVEC_INTEGER_BINARY_REQUIREMENT(NAME, OP)                   \
    [ForceInline] [require(cooperative_vector)]                                 \
    This NAME(This other)                                                       \
    {                                                                            \
        This result;                                                             \
        [ForceUnroll]                                                            \
        for (int index = 0; index < N; ++index)                                 \
            result[index] = this[index] OP other[index];                         \
        return result;                                                           \
    }

    NUMERICS_COOPVEC_INTEGER_BINARY_REQUIREMENT(bitwiseAnd, &)
    NUMERICS_COOPVEC_INTEGER_BINARY_REQUIREMENT(bitwiseOr, |)
    NUMERICS_COOPVEC_INTEGER_BINARY_REQUIREMENT(bitwiseXor, ^)

#undef NUMERICS_COOPVEC_INTEGER_BINARY_REQUIREMENT

    [ForceInline] [require(cooperative_vector)]
    This bitwiseNot()
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = ~this[index];
        return result;
    }
}

public extension<
    T : __BuiltinIntegerType & __BuiltinSignedArithmeticType & IScalarSignedIntegerType,
    let N : int>
    CoopVec<T, N> : ISignedIntegerType
{
    [ForceInline] [require(cooperative_vector)]
    This negated() { return this.neg(); }

    [ForceInline] [require(cooperative_vector)]
    This absoluteValue()
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = __numericInterfaceBuiltinAbs(this[index]);
        return result;
    }
}

public extension<T : __BuiltinIntegerType & IScalarUnsignedIntegerType, let N : int>
    CoopVec<T, N> : IUnsignedIntegerType
{}

public extension<T : __BuiltinFloatingPointType & IScalarFractional, let N : int>
    CoopVec<T, N> : IComponentwiseOrdered
{
    typealias Scalar = T;
    typealias Mask = vector<bool, N>;

    [ForceInline] [require(cooperative_vector)]
    static This fromScalar(Scalar value) { return This(value); }

    [ForceInline] [require(cooperative_vector)]
    Mask isEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] == other[index];
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    Mask isLessThan(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] < other[index];
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    Mask isLessThanOrEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] <= other[index];
        return result;
    }
}

public extension<T : __BuiltinFloatingPointType & IScalarFractional, let N : int>
    CoopVec<T, N> : IFractional
{
    [ForceInline] [require(cooperative_vector)]
    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromInteger<T>(value));
    }
    [ForceInline] [require(cooperative_vector)]
    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromFloatingPoint<T>(value));
    }
    [ForceInline] [require(cooperative_vector)]
    static This zero() { return This(Scalar.zero()); }
    [ForceInline] [require(cooperative_vector)]
    static This one() { return This(Scalar.one()); }

    [ForceInline] [require(cooperative_vector)]
    This adding(This other) { return this.add(other); }
    [ForceInline] [require(cooperative_vector)]
    This subtracting(This other) { return this.sub(other); }
    [ForceInline] [require(cooperative_vector)]
    This multipliedBy(This other) { return this.mul(other); }
    [ForceInline] [require(cooperative_vector)]
    This dividedBy(This other) { return this.div(other); }
    [ForceInline] [require(cooperative_vector)]
    This negated() { return this.neg(); }

    [ForceInline] [require(cooperative_vector)]
    This absoluteValue()
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = __numericInterfaceBuiltinAbs(this[index]);
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    This reciprocal()
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = __numericInterfaceBuiltinRcp(this[index]);
        return result;
    }
}

#define NUMERICS_COOPVEC_UNARY_REQUIREMENT(NAME, BUILTIN)                       \
    [ForceInline] [require(cooperative_vector)]                                 \
    This NAME()                                                                 \
    {                                                                            \
        This result;                                                             \
        [ForceUnroll]                                                            \
        for (int index = 0; index < N; ++index)                                 \
            result[index] = BUILTIN(this[index]);                                \
        return result;                                                           \
    }

#define NUMERICS_COOPVEC_BINARY_REQUIREMENT(NAME, BUILTIN)                      \
    [ForceInline] [require(cooperative_vector)]                                 \
    This NAME(This other)                                                       \
    {                                                                            \
        This result;                                                             \
        [ForceUnroll]                                                            \
        for (int index = 0; index < N; ++index)                                 \
            result[index] = BUILTIN(this[index], other[index]);                  \
        return result;                                                           \
    }

public extension<T : __BuiltinFloatingPointType & ITrigonometricFunctions, let N : int>
    CoopVec<T, N> : ITrigonometricFunctions
{
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(sin, __numericInterfaceBuiltinSin)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(cos, __numericInterfaceBuiltinCos)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(tan, __numericInterfaceBuiltinTan)

    [ForceInline] [require(cooperative_vector)]
    void sincos(out This sineResult, out This cosineResult)
    {
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
        {
            T scalarSine;
            T scalarCosine;
            __numericInterfaceBuiltinSinCos(this[index], scalarSine, scalarCosine);
            sineResult[index] = scalarSine;
            cosineResult[index] = scalarCosine;
        }
    }
}

public extension<T : __BuiltinFloatingPointType & IExponentialFunctions, let N : int>
    CoopVec<T, N> : IExponentialFunctions
{
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(exponential, __numericInterfaceBuiltinExp)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(exponentialWithBase2, __numericInterfaceBuiltinExp2)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(exponentialWithBase10, __numericInterfaceBuiltinExp10)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(logarithm, __numericInterfaceBuiltinLog)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(logarithmWithBase2, __numericInterfaceBuiltinLog2)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(logarithmWithBase10, __numericInterfaceBuiltinLog10)
    NUMERICS_COOPVEC_BINARY_REQUIREMENT(raisedToThePowerOf, __numericInterfaceBuiltinPow)
}

public extension<T : __BuiltinFloatingPointType & IInverseTrigonometricFunctions, let N : int>
    CoopVec<T, N> : IInverseTrigonometricFunctions
{
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(asin, __numericInterfaceBuiltinAsin)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(acos, __numericInterfaceBuiltinAcos)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(atan, __numericInterfaceBuiltinAtan)
    NUMERICS_COOPVEC_BINARY_REQUIREMENT(atan2, __numericInterfaceBuiltinAtan2)
}

public extension<T : __BuiltinFloatingPointType & IHyperbolicFunctions, let N : int>
    CoopVec<T, N> : IHyperbolicFunctions
{
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(sinh, __numericInterfaceBuiltinSinh)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(cosh, __numericInterfaceBuiltinCosh)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(tanh, __numericInterfaceBuiltinTanh)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(asinh, __numericInterfaceBuiltinAsinh)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(acosh, __numericInterfaceBuiltinAcosh)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(atanh, __numericInterfaceBuiltinAtanh)
}

public extension<T : __BuiltinFloatingPointType & IRootFunctions, let N : int>
    CoopVec<T, N> : IRootFunctions
{
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(squareRoot, __numericInterfaceBuiltinSqrt)
    NUMERICS_COOPVEC_UNARY_REQUIREMENT(reciprocalSquareRoot, __numericInterfaceBuiltinReciprocalSqrt)
}

#undef NUMERICS_COOPVEC_UNARY_REQUIREMENT
#undef NUMERICS_COOPVEC_BINARY_REQUIREMENT

public extension<T : __BuiltinFloatingPointType & IScalarReal, let N : int>
    CoopVec<T, N> : IRealOrderingFunctions
{
    [ForceInline] [require(cooperative_vector)]
    This minimum(This other)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = __numericsBuiltinMinimum(this[index], other[index]);
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    This maximum(This other)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = __numericsBuiltinMaximum(this[index], other[index]);
        return result;
    }

    [ForceInline] [require(cooperative_vector)]
    This step(This edge)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
        {
            result[index] =
                this[index] < edge[index] ? T.zero() : T.one();
        }
        return result;
    }
}
