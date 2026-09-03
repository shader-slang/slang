implementing numerics;

// These wrappers coexist with core-module overloads, and the compiler does not yet generally rank
// one generic overload above another from their interface refinements. Keep the ranks aligned with
// the existing core tiers:
//
// - Rank -10 is the ordinary interface-generic fallback. Unranked core math overloads and rank-10
//   builtin operators therefore continue to handle builtin types.
// - Rank -4 is immediately below the legacy IArithmetic operator tier at -3 (with IFloat at -2
//   and IDifferentiableArithmetic at -1), so existing generic constraints retain their operators.
// - Ranks -9 and -11 bracket the legacy IComparable adapters at -10. Scalar bool-valued relations
//   and min/max win at -9, while component-wise Mask-valued relations remain fallback behavior at
//   -11 for types that also conform to IComparable.

/// Returns true if every logical component is true.
__generic<M : IBooleanMask>
[__readNone]
[__unsafeForceInlineEarly]
public bool all(M value)
{
    return value.all();
}

/// Returns true if at least one logical component is true.
__generic<M : IBooleanMask>
[__readNone]
[__unsafeForceInlineEarly]
public bool any(M value)
{
    return value.any();
}

__generic<T : ISignedNumeric>
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T abs(T value)
{
    return value.absoluteValue();
}

__generic<T : IFractional>
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T rcp(T value)
{
    return value.reciprocal();
}

// The new bool-valued comparison contracts are independent of the legacy IComparable interface.
// Rank these overloads above the legacy and component-wise adapters so that a type conforming to
// more than one comparison vocabulary gets its explicit bool-valued relation in scalar contexts.
__generic<T : IEquatable>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator ==(T v0, T v1)
{
    return v0.isEqualTo(v1);
}

__generic<T : IEquatable>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator !=(T v0, T v1)
{
    return !v0.isEqualTo(v1);
}

__generic<T : IPartiallyOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator <(T v0, T v1)
{
    return v0.isLessThan(v1);
}

__generic<T : IPartiallyOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator >(T v0, T v1)
{
    return v1.isLessThan(v0);
}

__generic<T : IPartiallyOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator <=(T v0, T v1)
{
    return v0.isLessThanOrEqualTo(v1);
}

__generic<T : IPartiallyOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public bool operator >=(T v0, T v1)
{
    return v1.isLessThanOrEqualTo(v0);
}

// Component-wise comparison is a separate numeric-shape contract. Shaped comparisons return a
// Mask rather than reducing to one bool. Keep these overloads below the bool-valued adapters so a
// scalar type conforming to both selects the general relation, while generic code constrained only
// by a component-wise interface still selects the shape-preserving overloads.
__generic<T : IComponentwiseEquatable>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator ==(T v0, T v1)
{
    return v0.isEqualTo(v1);
}

__generic<T : IComponentwiseEquatable>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator !=(T v0, T v1)
{
    return v0.isEqualTo(v1).logicalNot();
}

__generic<T : IComponentwiseOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator <(T v0, T v1)
{
    return v0.isLessThan(v1);
}

__generic<T : IComponentwiseOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator <=(T v0, T v1)
{
    return v0.isLessThanOrEqualTo(v1);
}

__generic<T : IComponentwiseOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator >(T v0, T v1)
{
    return v1.isLessThan(v0);
}

__generic<T : IComponentwiseOrdered>
[__unsafeForceInlineEarly]
[OverloadRank(-11)]
public T.Mask operator >=(T v0, T v1)
{
    return v1.isLessThanOrEqualTo(v0);
}

// These overloads expose the new scalar-or-shaped numeric hierarchy without changing the existing
// IArithmetic, IFloat, or IDifferentiableArithmetic contracts. Keep them below the legacy builtin
// overloads so concrete builtin expressions continue to select their direct intrinsic operations;
// a generic parameter constrained only by the new interfaces still selects these overloads.
__generic<T : IAdditive>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator +(T v0, T v1)
{
    return v0.plus(v1);
}

__generic<T : IAdditive>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator -(T v0, T v1)
{
    return v0.minus(v1);
}

__generic<T : IAdditive>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __prefix T operator +(T value)
{
    return value;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator *(T v0, T v1)
{
    return v0.multipliedBy(v1);
}

__generic<T : ISignedNumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __prefix T operator -(T v0)
{
    return v0.negated();
}

__generic<T : IFractional>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator /(T v0, T v1)
{
    return v0.dividedBy(v1);
}

__generic<T : IFloatingPoint>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator %(T v0, T v1)
{
    return v0.remainderWhenDividedBy(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator /(T v0, T v1)
{
    return v0.quotientWhenDividedBy(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator %(T v0, T v1)
{
    return v0.remainderWhenDividedBy(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator &(T v0, T v1)
{
    return v0.bitwiseAnd(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator |(T v0, T v1)
{
    return v0.bitwiseOr(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator ^(T v0, T v1)
{
    return v0.bitwiseXor(v1);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __prefix T operator ~(T v0)
{
    return v0.bitwiseNot();
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator <<(T value, T.ShiftAmount amount)
{
    return value.shiftedLeftBy(amount);
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator >>(T value, T.ShiftAmount amount)
{
    return value.shiftedRightBy(amount);
}

// Derive compound assignment in these public wrappers instead of adding mutating interface
// requirements. Inherited default mutating methods do not yet lower reliably through a derived
// conformance (see issue #12814).
__generic<T : IAdditive>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator +=(in out T left, T right)
{
    left = left.plus(right);
    return left;
}

__generic<T : IAdditive>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator -=(in out T left, T right)
{
    left = left.minus(right);
    return left;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator *=(in out T left, T right)
{
    left = left.multipliedBy(right);
    return left;
}

__generic<T : IFractional>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator /=(in out T left, T right)
{
    left = left.dividedBy(right);
    return left;
}

__generic<T : IFloatingPoint>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator %=(in out T left, T right)
{
    left = left.remainderWhenDividedBy(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator /=(in out T left, T right)
{
    left = left.quotientWhenDividedBy(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator %=(in out T left, T right)
{
    left = left.remainderWhenDividedBy(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator &=(in out T left, T right)
{
    left = left.bitwiseAnd(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator |=(in out T left, T right)
{
    left = left.bitwiseOr(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator ^=(in out T left, T right)
{
    left = left.bitwiseXor(right);
    return left;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator <<=(in out T value, T.ShiftAmount amount)
{
    value = value.shiftedLeftBy(amount);
    return value;
}

__generic<T : IIntegerType>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public T operator >>=(in out T value, T.ShiftAmount amount)
{
    value = value.shiftedRightBy(amount);
    return value;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __prefix T operator ++(in out T value)
{
    value = value.plus(T.one());
    return value;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __postfix T operator ++(in out T value)
{
    let original = value;
    value = value.plus(T.one());
    return original;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __prefix T operator --(in out T value)
{
    value = value.minus(T.one());
    return value;
}

__generic<T : INumeric>
[__unsafeForceInlineEarly]
[OverloadRank(-4)]
public __postfix T operator --(in out T value)
{
    let original = value;
    value = value.minus(T.one());
    return original;
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T floor<T : IFloatingPoint>(T value)
{
    return value.floor();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T ceil<T : IFloatingPoint>(T value)
{
    return value.ceiling();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T round<T : IFloatingPoint>(T value)
{
    return value.rounded();
}

[__readNone]
[__unsafeForceInlineEarly]
[require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
[OverloadRank(-10)]
public T rint<T : IFloatingPoint>(T value)
{
    return value.roundedToNearestEven();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T trunc<T : IFloatingPoint>(T value)
{
    return value.truncated();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T frac<T : IFloatingPoint>(T value)
{
    return value.fractionalPart();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fract<T : IFloatingPoint>(T value)
{
    return value.fractionalPart();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmod<T : IFloatingPoint>(T value, T divisor)
{
    return value.remainderWhenDividedBy(divisor);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T copysign<T : IFloatingPoint>(T magnitude, T signSource)
{
    return magnitude.copyingSignFrom(signSource);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T modf<T : IFloatingPoint>(T value, out T integralPart)
{
    return value.fractionalPart(integralPart);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T.Mask isnan<T : IFloatingPoint>(T value)
{
    return value.isNaN();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T.Mask isfinite<T : IFloatingPoint>(T value)
{
    return value.isFinite();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T.Mask isinf<T : IFloatingPoint>(T value)
{
    return value.isInfinite();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T exp<T : IExponentialFunctions>(T value)
{
    return value.exponential();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T exp2<T : IExponentialFunctions>(T value)
{
    return value.exponentialWithBase2();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T exp10<T : IExponentialFunctions>(T value)
{
    return value.exponentialWithBase10();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T log<T : IExponentialFunctions>(T value)
{
    return value.logarithm();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T log2<T : IExponentialFunctions>(T value)
{
    return value.logarithmWithBase2();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T log10<T : IExponentialFunctions>(T value)
{
    return value.logarithmWithBase10();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T sin<T : ITrigonometricFunctions>(T value)
{
    return value.sin();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T cos<T : ITrigonometricFunctions>(T value)
{
    return value.cos();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T tan<T : ITrigonometricFunctions>(T value)
{
    return value.tan();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T asin<T : IInverseTrigonometricFunctions>(T value)
{
    return value.asin();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T acos<T : IInverseTrigonometricFunctions>(T value)
{
    return value.acos();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T atan<T : IInverseTrigonometricFunctions>(T value)
{
    return value.atan();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T atan2<T : IInverseTrigonometricFunctions>(T numerator, T denominator)
{
    return numerator.atan2(denominator);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T sinh<T : IHyperbolicFunctions>(T value)
{
    return value.sinh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T cosh<T : IHyperbolicFunctions>(T value)
{
    return value.cosh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T tanh<T : IHyperbolicFunctions>(T value)
{
    return value.tanh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T asinh<T : IHyperbolicFunctions>(T value)
{
    return value.asinh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T acosh<T : IHyperbolicFunctions>(T value)
{
    return value.acosh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T atanh<T : IHyperbolicFunctions>(T value)
{
    return value.atanh();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T pow<T : IExponentialFunctions>(T base, T exponent)
{
    return base.raisedToThePowerOf(exponent);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public void sincos<T : ITrigonometricFunctions>(T value, out T sineResult, out T cosineResult)
{
    value.sincos(sineResult, cosineResult);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T sqrt<T : IRootFunctions>(T value)
{
    return value.squareRoot();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T rsqrt<T : IRootFunctions>(T value)
{
    return value.reciprocalSquareRoot();
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public T min<T : IRealOrderingFunctions>(T left, T right)
{
    return left.minimum(right);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-9)]
public T max<T : IRealOrderingFunctions>(T left, T right)
{
    return left.maximum(right);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T clamp<T : IRealOrderingFunctions>(T value, T minimumValue, T maximumValue)
{
    return value.maximum(minimumValue).minimum(maximumValue);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T saturate<T : IFractional & IRealOrderingFunctions>(T value)
{
    return clamp<T>(
        value,
        T.zero(),
        T.one());
}

/// Returns the positive component-wise difference between `x` and `y`.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fdim<T : IFractional & IRealOrderingFunctions>(T x, T y)
{
    T zero = T.zero();
    return x.minus(y).maximum(zero);
}

/// Divides values that support fractional arithmetic.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T divide<T : IFractional>(T x, T y)
{
    return x.dividedBy(y);
}

/// Linearly interpolates between `x` and `y` by `factor`.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T lerp<T : IFractional>(T x, T y, T factor)
{
    return x + (y - x) * factor;
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T max3<T : IRealOrderingFunctions>(T x, T y, T z)
{
    return x.maximum(y.maximum(z));
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T min3<T : IRealOrderingFunctions>(T x, T y, T z)
{
    return x.minimum(y.minimum(z));
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T median3<T : IRealOrderingFunctions>(T x, T y, T z)
{
    T lower = y.minimum(z);
    T upper = y.maximum(z);
    return upper.minimum(x.maximum(lower));
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmax<T : IFloatingPoint & IRealOrderingFunctions>(T x, T y)
{
    return x.maximum(y);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmax3<T : IFloatingPoint & IRealOrderingFunctions>(T x, T y, T z)
{
    return x.maximum(y.maximum(z));
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmin<T : IFloatingPoint & IRealOrderingFunctions>(T x, T y)
{
    return x.minimum(y);
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmin3<T : IFloatingPoint & IRealOrderingFunctions>(T x, T y, T z)
{
    return x.minimum(y.minimum(z));
}

[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T fmedian3<T : IFloatingPoint & IRealOrderingFunctions>(T x, T y, T z)
{
    return median3(x, y, z);
}

/// Raises a positive base to a power through the elementary-function contract.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T powr<T : IExponentialFunctions>(T x, T y)
{
    return x.raisedToThePowerOf(y);
}

/// Applies a Hermite interpolation between zero and one.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T smoothstep<T : IFractional & IRealOrderingFunctions>(
    T minimumValue,
    T maximumValue,
    T value)
{
    T t = saturate((value - minimumValue) / (maximumValue - minimumValue));
    T one = T.one();
    T two = one + one;
    T three = two + one;
    return t * t * (three - two * t);
}

/// Returns zero where `value` is less than `edge`, and one otherwise.
[__readNone]
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
public T step<T : IRealOrderingFunctions>(T edge, T value)
{
    return value.step(edge);
}
