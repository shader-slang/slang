/**
Differentiable adapters from the experimental numeric contracts to established core operations.

@remarks This module is compiled without importing `slang.numerics`, so these bodies cannot route
back through the public interface-generic overloads. The compiler synthesizes derivatives from the
existing core registrations and serializes them for use by the differentiable annex.
*/
module __builtin_differentiable;

#define NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(NAME, METHOD)                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public T NAME<T : IFloat>(T left, T right)                                  \
    {                                                                            \
        return left.METHOD(right);                                               \
    }

NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsAdd, add)
NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsSub, sub)
NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsMul, mul)
NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsDiv, div)

#undef NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public T __differentiableNumericsNeg<T : IFloat>(T value)
{
    return value.neg();
}

#define NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(NAME, BUILTIN)                    \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public T NAME<T : __BuiltinFloatingPointType>(T value)                      \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }                                                                            \
                                                                                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public vector<T, N> NAME<T : __BuiltinFloatingPointType, let N : int>(      \
        vector<T, N> value)                                                      \
    {                                                                            \
        return ::BUILTIN(value);                                                 \
    }

NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAbs, abs)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAcos, acos)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAcosh, acosh)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAsin, asin)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAsinh, asinh)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAtan, atan)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAtanh, atanh)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsCos, cos)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsCosh, cosh)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsExp, exp)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsExp2, exp2)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsFractionalPart, frac)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog, log)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog10, log10)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog2, log2)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsRcp, rcp)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsRsqrt, rsqrt)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSin, sin)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSinh, sinh)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSqrt, sqrt)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsTan, tan)
NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsTanh, tanh)

#undef NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER

[__readNone]
[__unsafeForceInlineEarly]
public T __differentiableNumericsExp10<T : __BuiltinFloatingPointType>(T value)
{
    return ::exp10(value);
}

[__readNone]
[__unsafeForceInlineEarly]
public vector<T, N> __differentiableNumericsExp10<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value)
{
    return ::exp10(value);
}

#define NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(NAME, BUILTIN)                   \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public T NAME<T : __BuiltinFloatingPointType>(T left, T right)              \
    {                                                                            \
        return ::BUILTIN(left, right);                                           \
    }                                                                            \
                                                                                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public vector<T, N> NAME<T : __BuiltinFloatingPointType, let N : int>(      \
        vector<T, N> left,                                                       \
        vector<T, N> right)                                                      \
    {                                                                            \
        return ::BUILTIN(left, right);                                           \
    }

NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsAtan2, atan2)
NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsCopySign, copysign)
NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsMaximum, max)
NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsMinimum, min)
NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsPow, pow)
NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsRemainder, fmod)

#undef NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public void __differentiableNumericsSinCos<T : __BuiltinFloatingPointType>(
    T value,
    out T sineResult,
    out T cosineResult)
{
    ::sincos(value, sineResult, cosineResult);
}

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public void __differentiableNumericsSinCos<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value, out vector<T, N> sineResult, out vector<T, N> cosineResult)
{
    ::sincos(value, sineResult, cosineResult);
}
