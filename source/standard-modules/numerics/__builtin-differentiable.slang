// The differentiable conformances in `slang.numerics.differentiable` need to call the established
// core operations without resolving back to the public numeric wrappers that those conformances
// implement. This module defines adapters with distinguished names for that purpose. It does not
// import `slang.numerics`, so lookup in an adapter body cannot select a public numeric wrapper and
// form a cycle through its interface witness.
//
// The adapters are marked `[Differentiable]` so the compiler can use the core operation's existing
// derivative registration to synthesize the adapter's derivative. The serialized adapters and
// their derivatives are then available to the differentiable annex.
//
// This adapter layer should become unnecessary once Slang can bind an imported module as a named
// scope and directly call declarations such as `core::sin(value)` and the core `operator+`.
module __builtin_differentiable;

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public TDestination __differentiableNumericsConstructFromFloatingPoint<
    TDestination : __BuiltinFloatingPointType,
    TSource : __BuiltinFloatingPointType>(TSource value)
{
    return __realCast<TDestination>(value);
}

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public vector<TDestination, N> __differentiableNumericsConstructVectorFromFloatingPoint<
    TDestination : __BuiltinFloatingPointType,
    TSource : __BuiltinFloatingPointType,
    let N : int>(TSource value)
{
    return vector<TDestination, N>(__realCast<TDestination>(value));
}

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public matrix<TDestination, R, C, L> __differentiableNumericsConstructMatrixFromFloatingPoint<
    TDestination : __BuiltinFloatingPointType,
    TSource : __BuiltinFloatingPointType,
    let R : int,
    let C : int,
    let L : int>(TSource value)
{
    return matrix<TDestination, R, C, L>(__realCast<TDestination>(value));
}

// Core currently exposes differentiable component-wise arithmetic through the `add`, `sub`,
// `mul`, `div`, and `neg` members that satisfy `IFloat`. Keep that legacy interface out of these
// adapter signatures by spelling out the supported scalar, vector, and matrix shapes and
// constraining their element type instead. The bodies deliberately use the member operations:
// an ordinary matrix `operator*` can mean matrix multiplication rather than component-wise
// multiplication.
#define SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(NAME, METHOD)                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public T NAME<T : __BuiltinFloatingPointType>(T left, T right)              \
    {                                                                            \
        return left.METHOD(right);                                               \
    }                                                                            \
                                                                                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public vector<T, N> NAME<T : __BuiltinFloatingPointType, let N : int>(      \
        vector<T, N> left,                                                       \
        vector<T, N> right)                                                      \
    {                                                                            \
        return left.METHOD(right);                                               \
    }                                                                            \
                                                                                 \
    [__readNone] [Differentiable] [__unsafeForceInlineEarly]                    \
    public matrix<T, R, C, L> NAME<                                              \
        T : __BuiltinFloatingPointType,                                          \
        let R : int,                                                             \
        let C : int,                                                             \
        let L : int>(                                                            \
        matrix<T, R, C, L> left,                                                 \
        matrix<T, R, C, L> right)                                                \
    {                                                                            \
        return left.METHOD(right);                                               \
    }

SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsAdd, add)
SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsSub, sub)
SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsMul, mul)
SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER(__differentiableNumericsDiv, div)

#undef SLANG_NUMERICS_DIFFERENTIABLE_ARITHMETIC_ADAPTER

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public T __differentiableNumericsNeg<T : __BuiltinFloatingPointType>(T value)
{
    return value.neg();
}

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public vector<T, N> __differentiableNumericsNeg<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value)
{
    return value.neg();
}

[__readNone]
[Differentiable]
[__unsafeForceInlineEarly]
public matrix<T, R, C, L> __differentiableNumericsNeg<
    T : __BuiltinFloatingPointType,
    let R : int,
    let C : int,
    let L : int>(matrix<T, R, C, L> value)
{
    return value.neg();
}

// Matrix conformances apply these scalar and vector adapters component-wise so that they preserve
// the matrix layout parameter in their result type.
#define SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(NAME, BUILTIN)                    \
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

SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAbs, abs)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAcos, acos)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAcosh, acosh)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAsin, asin)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAsinh, asinh)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAtan, atan)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsAtanh, atanh)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsCos, cos)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsCosh, cosh)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsExp, exp)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsExp2, exp2)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsFractionalPart, frac)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog, log)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog10, log10)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsLog2, log2)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsRcp, rcp)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsRsqrt, rsqrt)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSin, sin)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSinh, sinh)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsSqrt, sqrt)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsTan, tan)
SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER(__differentiableNumericsTanh, tanh)

#undef SLANG_NUMERICS_DIFFERENTIABLE_UNARY_ADAPTER

// Core `exp10` does not currently provide the complete derivative registrations needed by an
// adapter marked `[Differentiable]`. The differentiable annex therefore attaches forward and
// backward derivatives to an annex-local forwarding function in `differentiable-exp10.slang`.
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

#define SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(NAME, BUILTIN)                   \
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

SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsAtan2, atan2)
SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsCopySign, copysign)
SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsMaximum, max)
SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsMinimum, min)
SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsPow, pow)
SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER(__differentiableNumericsRemainder, fmod)

#undef SLANG_NUMERICS_DIFFERENTIABLE_BINARY_ADAPTER

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
