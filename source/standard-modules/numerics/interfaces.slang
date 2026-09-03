implementing numerics;

/// Represents a scalar or shaped Boolean value that supports component-wise logical operations and
/// horizontal reductions.
public interface IBooleanMask
{
    /// Returns the component-wise logical negation.
    This logicalNot();

    /// Returns the component-wise logical conjunction with `other`.
    This logicalAnd(This other);

    /// Returns the component-wise logical disjunction with `other`.
    This logicalOr(This other);

    /// Returns true if any logical component is true.
    bool any();

    /// Returns true if every logical component is true.
    bool all();
}

/// Describes the scalar and comparison-result types of a numeric value.
public interface INumericShapedType
{
    /// The logical scalar repeated by this type. A logical scalar uses `Scalar == This`.
    associatedtype Scalar;

    /// The result type of component-wise comparison and classification operations.
    associatedtype Mask : IBooleanMask;

    /// Constructs a value whose every logical component is `value`.
    ///
    /// For a logical scalar, this operation returns `value` unchanged.
    static This fromScalar(Scalar value);
}

/// Refines `INumericShapedType` for a logical scalar.
public interface IScalarShapedType : INumericShapedType
{
    __constraint Scalar == This;
    __constraint Mask == bool;
}

/// Supports component-wise equality.
public interface IComponentwiseEquatable : INumericShapedType
{
    /// Compares this value with `other` and returns one Boolean result per logical component.
    Mask isEqualTo(This other);
}

/// Supports component-wise relational comparisons.
public interface IComponentwiseOrdered : IComponentwiseEquatable
{
    /// Tests whether each logical component is less than the corresponding component of `other`.
    Mask isLessThan(This other);

    /// Tests whether each logical component is less than or equal to the corresponding component
    /// of `other`.
    Mask isLessThanOrEqualTo(This other);
}

/// Represents a type with an equality relation.
///
/// This contract is independent of numeric scalar-or-shaped behavior.
public interface IEquatable
{
    /// Returns true when this value is equal to `other`.
    bool isEqualTo(This other);
}

/// Represents a type with a partial order.
///
/// This contract is independent of numeric scalar-or-shaped behavior. Numeric scalar refinements
/// compose it with `IScalarShapedType` and the applicable component-wise ordering interface.
public interface IPartiallyOrdered : IEquatable
{
    /// Returns true when this value is less than `other` according to the partial order.
    bool isLessThan(This other);

    /// Returns true when this value is less than or equal to `other` according to the partial
    /// order.
    bool isLessThanOrEqualTo(This other);
}

/// Represents a type with a total order, such as builtin integers.
public interface ITotallyOrdered : IPartiallyOrdered
{
}

/// Supports explicit construction from every builtin integer type.
public interface IConstructibleFromBuiltinInteger
{
    __init<TSource : __BuiltinIntegerType>(TSource value);
}

/// Supports explicit construction from every builtin floating-point type.
public interface IConstructibleFromBuiltinFloatingPoint
{
    __init<TSource : __BuiltinFloatingPointType>(TSource value);
}

/// Supports addition and subtraction for logical scalars and shaped values.
public interface IAdditive : INumericShapedType
{
    __constraint Scalar : IScalarAdditive;

    /// Returns the component-wise additive identity in this type's logical shape.
    static This zero();

    /// Returns the component-wise sum with `other`.
    This adding(This other);

    /// Returns the component-wise difference from `other`.
    This subtracting(This other);
}

/// Scalar refinement of `IAdditive`.
public interface IScalarAdditive : IAdditive, IScalarShapedType
{
}

/// Supports same-type multiplication in addition to additive arithmetic.
public interface INumeric : IAdditive, IConstructibleFromBuiltinInteger
{
    __constraint Scalar : IScalarNumeric;

    /// Returns the component-wise multiplicative identity in this type's logical shape.
    ///
    /// For a matrix, this is a matrix filled with ones rather than a matrix-algebra identity.
    static This one();

    /// Returns the component-wise product with `other`.
    This multipliedBy(This other);
}

/// Scalar refinement of `INumeric`.
public interface IScalarNumeric : INumeric, IScalarAdditive
{
}

/// Supports arithmetic negation.
public interface ISignedNumeric : INumeric
{
    __constraint Scalar : IScalarSignedNumeric;

    /// Returns the component-wise arithmetic negation.
    This negated();

    /// Returns the component-wise absolute value in the same numeric representation.
    ///
    /// For a signed integer's most-negative value, builtin types retain their existing `abs`
    /// behavior. A future full-range magnitude operation can return a distinct associated type.
    This absoluteValue();
}

/// Scalar refinement of `ISignedNumeric`.
public interface IScalarSignedNumeric : ISignedNumeric, IScalarNumeric
{
}

/// Supports bit shifts whose amount has the scalar or logical aggregate shape chosen by the
/// conformance.
public interface IBitShiftOperations
{
    associatedtype ShiftAmount;

    /// Returns this value shifted left component-wise by `amount`.
    This shiftedLeftBy(ShiftAmount amount);

    /// Returns this value shifted right component-wise by `amount`.
    This shiftedRightBy(ShiftAmount amount);
}

/// Supports ordinary same-shape integer division and bitwise operations.
///
/// This interface is scalar-or-shaped. Use `IScalarIntegerType` when an algorithm requires a
/// scalar integer rather than component-wise behavior.
public interface IIntegerType : INumeric, IComponentwiseOrdered, IBitShiftOperations
{
    __constraint Scalar : IScalarIntegerType;
    __constraint ShiftAmount : IIntegerType;

    /// Returns the component-wise integer quotient.
    This quotientWhenDividedBy(This divisor);

    /// Returns the component-wise integer remainder.
    This remainderWhenDividedBy(This divisor);

    /// Returns the component-wise bitwise conjunction with `other`.
    This bitwiseAnd(This other);

    /// Returns the component-wise bitwise disjunction with `other`.
    This bitwiseOr(This other);

    /// Returns the component-wise bitwise exclusive disjunction with `other`.
    This bitwiseXor(This other);

    /// Returns the component-wise bitwise complement.
    This bitwiseNot();
}

/// Scalar refinement of `IIntegerType`.
public interface IScalarIntegerType : IIntegerType, IScalarNumeric, ITotallyOrdered
{
}

/// Supports unsigned integer operations.
public interface IUnsignedIntegerType : IIntegerType
{
    __constraint Scalar : IScalarUnsignedIntegerType;
}

/// Supports signed integer operations.
public interface ISignedIntegerType : IIntegerType, ISignedNumeric
{
    __constraint Scalar : IScalarSignedIntegerType;
}

/// Scalar refinement of `IUnsignedIntegerType`.
public interface IScalarUnsignedIntegerType : IUnsignedIntegerType, IScalarIntegerType
{
}

/// Scalar refinement of `ISignedIntegerType`.
public interface IScalarSignedIntegerType
    : ISignedIntegerType, IScalarIntegerType, IScalarSignedNumeric
{
}

/// Supports signed arithmetic with division.
///
/// This interface does not imply a floating-point representation, elementary functions, or an
/// ordering. Fixed-point, ratio, dual-number, and interval types can conform when they provide the
/// required arithmetic behavior.
public interface IFractional : ISignedNumeric, IConstructibleFromBuiltinFloatingPoint
{
    __constraint Scalar : IScalarFractional;

    /// Returns the component-wise sum with `other`.
    This adding(This other);

    /// Returns the component-wise difference from `other`.
    This subtracting(This other);

    /// Returns the component-wise product with `other`.
    This multipliedBy(This other);

    /// Returns the component-wise quotient after division by `other`.
    This dividedBy(This other);

    /// Returns the component-wise arithmetic negation.
    This negated();

    /// Returns the component-wise absolute value in the same representation.
    This absoluteValue();

    /// Returns the component-wise reciprocal.
    This reciprocal();
}

/// Scalar refinement of `IFractional`.
public interface IScalarFractional : IFractional, IScalarSignedNumeric
{
}

/// Supports operations tied to a floating-point representation.
///
/// Elementary functions and ordering are separate capabilities.
public interface IFloatingPoint : IFractional
{
    __constraint Scalar : IScalarFloatingPoint;

    /// Returns the greatest integral value not greater than this value, component-wise.
    This floor();

    /// Returns the least integral value not less than this value, component-wise.
    This ceiling();

    /// Returns the nearest integral value, component-wise.
    ///
    /// A conformance may follow its target's convention for values exactly between two integers.
    This rounded();

    /// Returns the nearest integral value with ties rounded to even, component-wise.
    ///
    /// This operation is unavailable on targets where Slang does not currently expose `rint`.
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven();

    /// Discards the fractional part, component-wise.
    This truncated();

    /// Returns the fractional part, component-wise.
    This fractionalPart();

    /// Returns the component-wise floating-point remainder after truncating the quotient.
    This remainderWhenDividedBy(This divisor);

    /// Returns the magnitude of this value with the component-wise sign of `signSource`.
    This copyingSignFrom(This signSource);

    /// Splits this value into component-wise integral and fractional parts.
    ///
    /// The integral part is written to `integralPart`, and the fractional part is returned.
    This fractionalPart(out This integralPart);

    /// Classifies NaN values component-wise.
    Mask isNaN();

    /// Classifies finite values component-wise.
    Mask isFinite();

    /// Classifies infinite values component-wise.
    Mask isInfinite();
}

/// Scalar refinement of `IFloatingPoint`.
public interface IScalarFloatingPoint : IFloatingPoint, IScalarFractional
{
}

/// Supports exponential, logarithmic, and power functions without implying arithmetic or ordering.
public interface IExponentialFunctions
{
    /// Computes the base-e exponential component-wise.
    This exponential();

    /// Computes the base-2 exponential component-wise.
    This exponentialWithBase2();

    /// Computes the base-10 exponential component-wise.
    This exponentialWithBase10();

    /// Computes the natural logarithm component-wise.
    This logarithm();

    /// Computes the base-2 logarithm component-wise.
    This logarithmWithBase2();

    /// Computes the base-10 logarithm component-wise.
    This logarithmWithBase10();

    /// Raises this value to `exponent` component-wise.
    This raisedToThePowerOf(This exponent);
}

/// Supports trigonometric functions without implying arithmetic or ordering.
public interface ITrigonometricFunctions
{
    /// Computes the sine component-wise.
    This sin();

    /// Computes the cosine component-wise.
    This cos();

    /// Computes the tangent component-wise.
    This tan();

    /// Computes the sine and cosine component-wise in one operation.
    void sincos(out This sineResult, out This cosineResult);
}

/// Supports inverse trigonometric functions without implying arithmetic or ordering.
public interface IInverseTrigonometricFunctions
{
    /// Computes the arc sine component-wise.
    This asin();

    /// Computes the arc cosine component-wise.
    This acos();

    /// Computes the arc tangent component-wise.
    This atan();

    /// Computes the arc tangent of this value divided by `denominator` component-wise.
    This atan2(This denominator);
}

/// Supports hyperbolic and inverse hyperbolic functions without implying arithmetic or ordering.
public interface IHyperbolicFunctions
{
    /// Computes the hyperbolic sine component-wise.
    This sinh();

    /// Computes the hyperbolic cosine component-wise.
    This cosh();

    /// Computes the hyperbolic tangent component-wise.
    This tanh();

    /// Computes the inverse hyperbolic sine component-wise.
    This asinh();

    /// Computes the inverse hyperbolic cosine component-wise.
    This acosh();

    /// Computes the inverse hyperbolic tangent component-wise.
    This atanh();
}

/// Supports square-root functions without implying arithmetic or ordering.
public interface IRootFunctions
{
    /// Computes the square root component-wise.
    This squareRoot();

    /// Computes the reciprocal square root component-wise.
    This reciprocalSquareRoot();
}

/// Supports the standard families of elementary functions without imposing arithmetic or order.
public typealias IElementaryFunctions =
    IExponentialFunctions &
    ITrigonometricFunctions &
    IInverseTrigonometricFunctions &
    IHyperbolicFunctions &
    IRootFunctions;

/// Scalar refinement of `IElementaryFunctions`.
public typealias IScalarElementaryFunctions = IElementaryFunctions & IScalarShapedType;

/// Supports operations whose conventional definitions depend on a real ordering.
public interface IRealOrderingFunctions
{
    /// Computes the component-wise minimum.
    This minimum(This other);

    /// Computes the component-wise maximum.
    This maximum(This other);

    /// Returns the component-wise step result: zero where this value is less than `edge`, and one
    /// otherwise.
    This step(This edge);
}

/// Supports fractional arithmetic, elementary functions, and component-wise real ordering.
public typealias IReal =
    IFractional &
    IElementaryFunctions &
    IComponentwiseOrdered &
    IRealOrderingFunctions;

/// Scalar refinement of `IReal`.
public typealias IScalarReal = IReal & IScalarFractional & IPartiallyOrdered;
