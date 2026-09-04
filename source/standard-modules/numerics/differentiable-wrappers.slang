implementing differentiable;

// Consider a differentiable generic such as this one:
//
//     [Differentiable]
//     T wave<T : IDifferentiableTrigonometricFunctions>(T value)
//     {
//         return sin(value);
//     }
//
// This file exposes conventional operators and free functions for the differentiable interface
// requirements. Importing `slang.numerics.differentiable` makes these overloads available, and
// declarations in this module are found before the corresponding declarations brought in by its
// exported import of `slang.numerics`. The free functions intentionally use the base module's -10
// fallback rank, while the operators use its -4 fallback rank. Those negative ranks keep concrete
// and legacy core overloads preferred; raising only selected differentiable wrappers would make the
// overload policy depend on incidental operation-by-operation exceptions. The bodies dispatch to
// differentiable requirements so that builtin and user-defined conformances can use spellings such
// as `sin(value)` instead of `value.sin()` without making differentiability part of the base
// contracts.

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T abs<T : IDifferentiableFractional>(T value)
{
    return value.absoluteValue();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T rcp<T : IDifferentiableFractional>(T value)
{
    return value.reciprocal();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T frac<T : IDifferentiableFloatingPoint>(T value)
{
    return value.fractionalPart();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T fract<T : IDifferentiableFloatingPoint>(T value)
{
    return value.fractionalPart();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T fmod<T : IDifferentiableFloatingPoint>(T value, T divisor)
{
    return value.remainderWhenDividedBy(divisor);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T copysign<T : IDifferentiableFloatingPoint>(T magnitude, T signSource)
{
    return magnitude.copyingSignFrom(signSource);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T exp<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.exponential();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T exp2<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.exponentialWithBase2();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T exp10<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.exponentialWithBase10();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T log<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.logarithm();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T log2<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.logarithmWithBase2();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T log10<T : IDifferentiableExponentialFunctions>(T value)
{
    return value.logarithmWithBase10();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T pow<T : IDifferentiableExponentialFunctions>(T base, T exponent)
{
    return base.raisedToThePowerOf(exponent);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T sin<T : IDifferentiableTrigonometricFunctions>(T value)
{
    return value.sin();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T cos<T : IDifferentiableTrigonometricFunctions>(T value)
{
    return value.cos();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T tan<T : IDifferentiableTrigonometricFunctions>(T value)
{
    return value.tan();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public void sincos<T : IDifferentiableTrigonometricFunctions>(
    T value,
    out T sineResult,
    out T cosineResult)
{
    value.sincos(sineResult, cosineResult);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T asin<T : IDifferentiableInverseTrigonometricFunctions>(T value)
{
    return value.asin();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T acos<T : IDifferentiableInverseTrigonometricFunctions>(T value)
{
    return value.acos();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T atan<T : IDifferentiableInverseTrigonometricFunctions>(T value)
{
    return value.atan();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T atan2<T : IDifferentiableInverseTrigonometricFunctions>(T numerator, T denominator)
{
    return numerator.atan2(denominator);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T sinh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.sinh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T cosh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.cosh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T tanh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.tanh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T asinh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.asinh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T acosh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.acosh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T atanh<T : IDifferentiableHyperbolicFunctions>(T value)
{
    return value.atanh();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T sqrt<T : IDifferentiableRootFunctions>(T value)
{
    return value.squareRoot();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T rsqrt<T : IDifferentiableRootFunctions>(T value)
{
    return value.reciprocalSquareRoot();
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T min<T : IDifferentiableRealOrderingFunctions>(T left, T right)
{
    return left.minimum(right);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T max<T : IDifferentiableRealOrderingFunctions>(T left, T right)
{
    return left.maximum(right);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T clamp<T : IDifferentiableRealOrderingFunctions>(
    T value,
    T minimumValue,
    T maximumValue)
{
    return value.maximum(minimumValue).minimum(maximumValue);
}

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T saturate<T : IDifferentiableFractional & IDifferentiableRealOrderingFunctions>(T value)
{
    // Identity construction is intentionally outside the differentiable contract. These values
    // are constants, so no derivative needs to flow through their construction.
    T zero = no_diff(T.zero());
    T one = no_diff(T.one());
    return value.maximum(zero).minimum(one);
}

/// Returns the differentiable positive component-wise difference between `x` and `y`.
[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T fdim<T : IDifferentiableFractional & IDifferentiableRealOrderingFunctions>(T x, T y)
{
    T zero = no_diff(T.zero());
    return x.minus(y).maximum(zero);
}

/// Divides values through a differentiable fractional-arithmetic contract.
[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T divide<T : IDifferentiableFractional>(T x, T y)
{
    return x.dividedBy(y);
}

/// Linearly interpolates through a differentiable fractional-arithmetic contract.
[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T lerp<T : IDifferentiableFractional>(T x, T y, T factor)
{
    return x.plus(y.minus(x).multipliedBy(factor));
}

#define SLANG_NUMERICS_DIFFERENTIABLE_THREE_WAY_EXTREMUM(NAME, BODY)                  \
    [__readNone] [Differentiable] [ForceInline] [OverloadRank(-10)]            \
    public T NAME<T : IDifferentiableRealOrderingFunctions>(T x, T y, T z)     \
    {                                                                            \
        BODY                                                                     \
    }

SLANG_NUMERICS_DIFFERENTIABLE_THREE_WAY_EXTREMUM(max3, return x.maximum(y.maximum(z));)
SLANG_NUMERICS_DIFFERENTIABLE_THREE_WAY_EXTREMUM(min3, return x.minimum(y.minimum(z));)
SLANG_NUMERICS_DIFFERENTIABLE_THREE_WAY_EXTREMUM(
    median3,
    T lower = y.minimum(z); T upper = y.maximum(z); return upper.minimum(x.maximum(lower));)

#undef SLANG_NUMERICS_DIFFERENTIABLE_THREE_WAY_EXTREMUM

#define SLANG_NUMERICS_DIFFERENTIABLE_FLOATING_EXTREMUM(NAME, BODY)                  \
    [__readNone] [Differentiable] [ForceInline] [OverloadRank(-10)]            \
    public T NAME<                                                              \
        T : IDifferentiableFloatingPoint & IDifferentiableRealOrderingFunctions>( \
        T x, T y, T z)                                                          \
    {                                                                            \
        BODY                                                                     \
    }

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T fmax<
    T : IDifferentiableFloatingPoint & IDifferentiableRealOrderingFunctions>(T x, T y)
{
    return x.maximum(y);
}

SLANG_NUMERICS_DIFFERENTIABLE_FLOATING_EXTREMUM(fmax3, return x.maximum(y.maximum(z));)

[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T fmin<
    T : IDifferentiableFloatingPoint & IDifferentiableRealOrderingFunctions>(T x, T y)
{
    return x.minimum(y);
}

SLANG_NUMERICS_DIFFERENTIABLE_FLOATING_EXTREMUM(fmin3, return x.minimum(y.minimum(z));)
SLANG_NUMERICS_DIFFERENTIABLE_FLOATING_EXTREMUM(
    fmedian3,
    T lower = y.minimum(z); T upper = y.maximum(z); return upper.minimum(x.maximum(lower));)

#undef SLANG_NUMERICS_DIFFERENTIABLE_FLOATING_EXTREMUM

/// Raises a positive base through the differentiable exponential-function contract.
[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T powr<T : IDifferentiableExponentialFunctions>(T x, T y)
{
    return x.raisedToThePowerOf(y);
}

/// Applies differentiable Hermite interpolation between zero and one.
[__readNone]
[Differentiable]
[ForceInline]
[OverloadRank(-10)]
public T smoothstep<
    T : IDifferentiableFractional & IDifferentiableRealOrderingFunctions>(
    T minimumValue,
    T maximumValue,
    T value)
{
    T zero = no_diff(T.zero());
    T one = no_diff(T.one());
    T t = value.minus(minimumValue).dividedBy(maximumValue.minus(minimumValue));
    t = t.maximum(zero).minimum(one);
    T two = one.plus(one);
    T three = two.plus(one);
    return t.multipliedBy(t).multipliedBy(three.minus(two.multipliedBy(t)));
}

__generic<T : IDifferentiableFractional>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public T operator +(T left, T right)
{
    return left.plus(right);
}

__generic<T : IDifferentiableFractional>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public T operator -(T left, T right)
{
    return left.minus(right);
}

__generic<T : IDifferentiableFractional>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public T operator *(T left, T right)
{
    return left.multipliedBy(right);
}

__generic<T : IDifferentiableFractional>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public T operator /(T left, T right)
{
    return left.dividedBy(right);
}

__generic<T : IDifferentiableFloatingPoint>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public T operator %(T left, T right)
{
    return left.remainderWhenDividedBy(right);
}

__generic<T : IDifferentiableFractional>
[Differentiable]
[ForceInline]
[OverloadRank(-4)]
public __prefix T operator -(T value)
{
    return value.negated();
}
