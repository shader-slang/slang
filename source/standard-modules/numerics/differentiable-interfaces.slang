implementing differentiable;

/// Supports differentiable signed arithmetic with division.
///
/// This interface refines the representation-independent `IFractional` contract without making
/// differentiability a cost of using that base interface. It re-declares the operations that Slang
/// can differentiate for builtin floating-point scalar and shaped types.
public interface IDifferentiableFractional : IFractional, IDifferentiable
{
    __constraint Scalar : IScalarDifferentiableFractional;

    /// Constructs a value from any builtin floating-point representation.
    [Differentiable]
    __init<TSource : __BuiltinFloatingPointType>(TSource value);

    [Differentiable]
    This adding(This other);

    [Differentiable]
    This subtracting(This other);

    [Differentiable]
    This multipliedBy(This other);

    [Differentiable]
    This dividedBy(This other);

    [Differentiable]
    This negated();

    [Differentiable]
    This absoluteValue();

    [Differentiable]
    This reciprocal();
}

/// Scalar refinement of `IDifferentiableFractional`.
public interface IScalarDifferentiableFractional
    : IDifferentiableFractional, IScalarFractional
{
}

/// Supports the differentiable subset of floating-point representation operations.
///
/// Operations such as `floor`, `round`, and classification remain available through
/// `IFloatingPoint`, but this interface does not claim they are differentiable.
public interface IDifferentiableFloatingPoint : IFloatingPoint, IDifferentiableFractional
{
    __constraint Scalar : IScalarDifferentiableFloatingPoint;

    [Differentiable]
    This fractionalPart();

    [Differentiable]
    This remainderWhenDividedBy(This divisor);

    [Differentiable]
    This copyingSignFrom(This signSource);
}

/// Scalar refinement of `IDifferentiableFloatingPoint`.
public interface IScalarDifferentiableFloatingPoint
    : IDifferentiableFloatingPoint, IScalarFloatingPoint, IScalarDifferentiableFractional
{
}

/// Supports differentiable exponential, logarithmic, and power functions.
public interface IDifferentiableExponentialFunctions : IExponentialFunctions, IDifferentiable
{
    [Differentiable]
    This exponential();

    [Differentiable]
    This exponentialWithBase2();

    [Differentiable]
    This exponentialWithBase10();

    [Differentiable]
    This logarithm();

    [Differentiable]
    This logarithmWithBase2();

    [Differentiable]
    This logarithmWithBase10();

    [Differentiable]
    This raisedToThePowerOf(This exponent);
}

/// Supports differentiable trigonometric functions.
public interface IDifferentiableTrigonometricFunctions : ITrigonometricFunctions, IDifferentiable
{
    [Differentiable]
    This sin();

    [Differentiable]
    This cos();

    [Differentiable]
    This tan();

    [Differentiable]
    void sincos(out This sineResult, out This cosineResult);
}

/// Supports differentiable inverse trigonometric functions.
public interface IDifferentiableInverseTrigonometricFunctions
    : IInverseTrigonometricFunctions, IDifferentiable
{
    [Differentiable]
    This asin();

    [Differentiable]
    This acos();

    [Differentiable]
    This atan();

    [Differentiable]
    This atan2(This denominator);
}

/// Supports differentiable hyperbolic and inverse hyperbolic functions.
public interface IDifferentiableHyperbolicFunctions : IHyperbolicFunctions, IDifferentiable
{
    [Differentiable]
    This sinh();

    [Differentiable]
    This cosh();

    [Differentiable]
    This tanh();

    [Differentiable]
    This asinh();

    [Differentiable]
    This acosh();

    [Differentiable]
    This atanh();
}

/// Supports differentiable square-root functions.
public interface IDifferentiableRootFunctions : IRootFunctions, IDifferentiable
{
    [Differentiable]
    This squareRoot();

    [Differentiable]
    This reciprocalSquareRoot();
}

/// Supports the differentiable variants of all elementary-function families.
public typealias IDifferentiableElementaryFunctions =
    IDifferentiableExponentialFunctions &
    IDifferentiableTrigonometricFunctions &
    IDifferentiableInverseTrigonometricFunctions &
    IDifferentiableHyperbolicFunctions &
    IDifferentiableRootFunctions;

/// Scalar refinement of `IDifferentiableElementaryFunctions`.
public typealias IScalarDifferentiableElementaryFunctions =
    IDifferentiableElementaryFunctions & IScalarShapedType;

/// Supports differentiable operations that depend on a real ordering.
///
/// `step` remains available through `IRealOrderingFunctions` without a differentiability claim.
public interface IDifferentiableRealOrderingFunctions : IRealOrderingFunctions, IDifferentiable
{
    [Differentiable]
    This minimum(This other);

    [Differentiable]
    This maximum(This other);
}

/// Supports differentiable fractional arithmetic, elementary functions, and real-ordering
/// operations.
public typealias IDifferentiableReal =
    IDifferentiableFractional &
    IDifferentiableElementaryFunctions &
    IComponentwiseOrdered &
    IDifferentiableRealOrderingFunctions;

/// Scalar refinement of `IDifferentiableReal`.
public typealias IScalarDifferentiableReal =
    IDifferentiableReal & IScalarDifferentiableFractional & IPartiallyOrdered;
