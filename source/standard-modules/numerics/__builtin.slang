/**
Adapters from the experimental numeric contracts to existing builtin operations.

@remarks This module is an implementation detail of `slang.numerics`. It is compiled before the
public numeric wrappers, so lookup in these bodies can only select the established core overloads.
Keeping this boundary avoids recursive dispatch without copying target-specific intrinsic bodies.
*/
module __builtin;

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAny(bool value)
{
    return value;
}

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAll(bool value)
{
    return value;
}

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAny<let N : int>(vector<bool, N> value)
{
    return ::any(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAll<let N : int>(vector<bool, N> value)
{
    return ::all(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAny<let R : int, let C : int, let L : int>(
    matrix<bool, R, C, L> value)
{
    [ForceUnroll]
    for (int row = 0; row < R; ++row)
    {
        [ForceUnroll]
        for (int column = 0; column < C; ++column)
        {
            if (value[row][column])
                return true;
        }
    }
    return false;
}

[__readNone]
[__unsafeForceInlineEarly]
public bool __numericsBuiltinAll<let R : int, let C : int, let L : int>(
    matrix<bool, R, C, L> value)
{
    [ForceUnroll]
    for (int row = 0; row < R; ++row)
    {
        [ForceUnroll]
        for (int column = 0; column < C; ++column)
        {
            if (!value[row][column])
                return false;
        }
    }
    return true;
}

[__unsafeForceInlineEarly]
public T __numericsLegacyAdd<T : IArithmetic>(T left, T right)
{
    return left.add(right);
}

[__unsafeForceInlineEarly]
public T __numericsLegacySub<T : IArithmetic>(T left, T right)
{
    return left.sub(right);
}

[__unsafeForceInlineEarly]
public T __numericsLegacyMul<T : IArithmetic>(T left, T right)
{
    return left.mul(right);
}

[__unsafeForceInlineEarly]
public T __numericsLegacyDiv<T : IArithmetic>(T left, T right)
{
    return left.div(right);
}

[__unsafeForceInlineEarly]
public T __numericsLegacyMod<T : IArithmetic>(T left, T right)
{
    return left.mod(right);
}

[__unsafeForceInlineEarly]
public T __numericsLegacyNeg<T : IArithmetic>(T value)
{
    return value.neg();
}

#define NUMERICS_SCALAR_COMPARISON_ADAPTER(NAME, OP)                            \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public bool NAME<T : __BuiltinArithmeticType>(T left, T right)              \
    {                                                                            \
        return left OP right;                                                    \
    }

#define NUMERICS_VECTOR_COMPARISON_ADAPTER(NAME, OP)                            \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public vector<bool, N> NAME<T : __BuiltinArithmeticType, let N : int>(      \
        vector<T, N> left,                                                       \
        vector<T, N> right)                                                      \
    {                                                                            \
        return left OP right;                                                    \
    }

NUMERICS_SCALAR_COMPARISON_ADAPTER(__numericsBuiltinEqual, ==)
NUMERICS_SCALAR_COMPARISON_ADAPTER(__numericsBuiltinLess, <)
NUMERICS_SCALAR_COMPARISON_ADAPTER(__numericsBuiltinLessEqual, <=)
NUMERICS_VECTOR_COMPARISON_ADAPTER(__numericsBuiltinEqual, ==)
NUMERICS_VECTOR_COMPARISON_ADAPTER(__numericsBuiltinLess, <)
NUMERICS_VECTOR_COMPARISON_ADAPTER(__numericsBuiltinLessEqual, <=)

#undef NUMERICS_VECTOR_COMPARISON_ADAPTER
#undef NUMERICS_SCALAR_COMPARISON_ADAPTER

#define NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(NAME, OP)                        \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public vector<T, N> NAME<T : __BuiltinIntegerType, let N : int>(            \
        vector<T, N> left,                                                       \
        vector<T, N> right)                                                      \
    {                                                                            \
        return left OP right;                                                    \
    }

#define NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(NAME, OP)                        \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public T NAME<T : __BuiltinIntegerType>(T left, T right)                    \
    {                                                                            \
        return left OP right;                                                    \
    }

NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(__numericsBuiltinIntegerDivide, /)
NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(__numericsBuiltinIntegerRemainder, %)
NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(__numericsBuiltinBitAnd, &)
NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(__numericsBuiltinBitOr, |)
NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER(__numericsBuiltinBitXor, ^)
NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(__numericsBuiltinIntegerDivide, /)
NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(__numericsBuiltinIntegerRemainder, %)
NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(__numericsBuiltinBitAnd, &)
NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(__numericsBuiltinBitOr, |)
NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER(__numericsBuiltinBitXor, ^)

#undef NUMERICS_INTEGER_VECTOR_BINARY_ADAPTER
#undef NUMERICS_INTEGER_SCALAR_BINARY_ADAPTER

[__readNone]
[__unsafeForceInlineEarly]
public T __numericsBuiltinShiftLeft<T : __BuiltinIntegerType>(T value, int amount)
{
    return value.shl(amount);
}

[__readNone]
[__unsafeForceInlineEarly]
public T __numericsBuiltinShiftRight<T : __BuiltinIntegerType>(T value, int amount)
{
    return value.shr(amount);
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericsBuiltinShiftLeft<
    T : __BuiltinIntegerType,
    let N : int>(vector<T, N> value, vector<int, N> amount)
{
    return value << amount;
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericsBuiltinShiftRight<
    T : __BuiltinIntegerType,
    let N : int>(vector<T, N> value, vector<int, N> amount)
{
    return value >> amount;
}

[__readNone]
[__unsafeForceInlineEarly]
public T __numericsBuiltinBitNot<T : __BuiltinIntegerType>(T value)
{
    return ~value;
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericsBuiltinBitNot<T : __BuiltinIntegerType, let N : int>(
    vector<T, N> value)
{
    return ~value;
}

#define NUMERICS_UNARY_ADAPTER(NAME, BUILTIN, ELEMENT_CONSTRAINT)                \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public T NAME<T : ELEMENT_CONSTRAINT>(T value)                               \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }                                                                            \
                                                                                 \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public vector<T, N> NAME<T : ELEMENT_CONSTRAINT, let N : int>(              \
        vector<T, N> value)                                                      \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }

NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinRcp, rcp, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAcos, acos, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAcosh, acosh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAsin, asin, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAsinh, asinh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAtan, atan, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinAtanh, atanh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinCeil, ceil, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinCos, cos, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinCosh, cosh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinExp, exp, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinExp10, exp10, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinExp2, exp2, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinFloor, floor, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinFractionalPart, frac, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinLog, log, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinLog10, log10, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinLog2, log2, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinReciprocalSqrt, rsqrt, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinRound, round, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinRoundEven, rint, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinSin, sin, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinSinh, sinh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinSqrt, sqrt, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinTan, tan, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinTanh, tanh, __BuiltinFloatingPointType)
NUMERICS_UNARY_ADAPTER(__numericInterfaceBuiltinTruncate, trunc, __BuiltinFloatingPointType)

#undef NUMERICS_UNARY_ADAPTER

[__readNone]
[__unsafeForceInlineEarly]
public T __numericInterfaceBuiltinAbs<
    T : __BuiltinIntegerType & __BuiltinSignedArithmeticType>(T value)
{
    return ::abs(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericInterfaceBuiltinAbs<
    T : __BuiltinIntegerType & __BuiltinSignedArithmeticType,
    let N : int>(vector<T, N> value)
{
    return ::abs(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public T __numericInterfaceBuiltinAbs<T : __BuiltinFloatingPointType>(T value)
{
    return ::abs(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericInterfaceBuiltinAbs<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value)
{
    return ::abs(value);
}

#define NUMERICS_BINARY_ADAPTER(NAME, BUILTIN)                                  \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public T NAME<T : __BuiltinFloatingPointType>(T left, T right)              \
    {                                                                            \
        return ::BUILTIN(left, right);                                           \
    }                                                                            \
                                                                                 \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public vector<T, N> NAME<T : __BuiltinFloatingPointType, let N : int>(      \
        vector<T, N> left,                                                       \
        vector<T, N> right)                                                      \
    {                                                                            \
        return ::BUILTIN(left, right);                                           \
    }

NUMERICS_BINARY_ADAPTER(__numericInterfaceBuiltinAtan2, atan2)
NUMERICS_BINARY_ADAPTER(__numericInterfaceBuiltinCopySign, copysign)
NUMERICS_BINARY_ADAPTER(__numericInterfaceBuiltinPow, pow)
NUMERICS_BINARY_ADAPTER(__numericInterfaceBuiltinRemainder, fmod)
NUMERICS_BINARY_ADAPTER(__numericsBuiltinMaximum, max)
NUMERICS_BINARY_ADAPTER(__numericsBuiltinMinimum, min)

#undef NUMERICS_BINARY_ADAPTER

#define NUMERICS_CLASSIFICATION_ADAPTER(NAME, BUILTIN)                          \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public bool NAME<T : __BuiltinFloatingPointType>(T value)                   \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }                                                                            \
                                                                                 \
    [__readNone] [__unsafeForceInlineEarly]                                     \
    public vector<bool, N> NAME<T : __BuiltinFloatingPointType, let N : int>(   \
        vector<T, N> value)                                                      \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }

NUMERICS_CLASSIFICATION_ADAPTER(__numericInterfaceBuiltinIsFinite, isfinite)
NUMERICS_CLASSIFICATION_ADAPTER(__numericInterfaceBuiltinIsInfinite, isinf)
NUMERICS_CLASSIFICATION_ADAPTER(__numericInterfaceBuiltinIsNaN, isnan)

#undef NUMERICS_CLASSIFICATION_ADAPTER

[__readNone]
[__unsafeForceInlineEarly]
public T __numericInterfaceBuiltinSplitFraction<T : __BuiltinFloatingPointType>(
    T value,
    out T integralPart)
{
    return ::modf(value, integralPart);
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __numericInterfaceBuiltinSplitFraction<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value, out vector<T, N> integralPart)
{
    return ::modf(value, integralPart);
}

[__readNone]
[__unsafeForceInlineEarly]
public void __numericInterfaceBuiltinSinCos<T : __BuiltinFloatingPointType>(
    T value,
    out T sineResult,
    out T cosineResult)
{
    ::sincos(value, sineResult, cosineResult);
}

[__readNone]
[__unsafeForceInlineEarly]
public void __numericInterfaceBuiltinSinCos<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value, out vector<T, N> sineResult, out vector<T, N> cosineResult)
{
    ::sincos(value, sineResult, cosineResult);
}
