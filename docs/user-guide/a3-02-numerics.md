---
layout: user-guide
---

# Generic Numerics

> **Note:** The interfaces in this chapter belong to experimental standard modules.
> Their names, decomposition, and requirements may change in future releases.

The `slang.numerics` module provides interfaces for generic algorithms that work with builtin and user-defined numeric types.
Its broad interfaces cover both scalar values and shaped values such as vectors and matrices, with component-wise semantics.
Narrow scalar refinements let an algorithm require a scalar when shaped behavior would not make sense.

## Enabling the Module

Import the module explicitly:

```slang
import slang.numerics;
```

Because the module is experimental, compile the program with `-experimental-feature`.
Importing `slang.numerics` does not change the interfaces or overloads provided by the core module.

## Writing Scalar-or-Shaped Algorithms

`IAdditive`, `INumeric`, and `ISignedNumeric` successively add component-wise addition and subtraction, multiplication, and signed operations.
A generic function constrained by one of these interfaces can use ordinary operator syntax.

For example, the following function works with builtin integer and floating-point scalars, vectors, and supported matrices:

```slang
T squarePlusOne<T : INumeric>(T value)
{
    let one = T(1);
    return value * value + one;
}

void useSquarePlusOne()
{
    let scalarResult = squarePlusOne(2.0);
    let vectorResult = squarePlusOne(float3(1.0, 2.0, 3.0));
}
```

For shaped values, these operators are component-wise.
In particular, multiplication through `INumeric` is not matrix multiplication.

`INumeric` guarantees explicit construction from every builtin integer type.
`IFractional` additionally guarantees explicit construction from every builtin floating-point type.
These contracts apply to values in general, not only to literals.
They make familiar cases such as `T(1)` and `T(0.5)` available while also supporting generic conversion from values whose types are `int8_t`, `uint64_t`, `half`, `double`, or another builtin numeric representation.

```slang
T convertNarrowInteger<T : INumeric>(int8_t value)
{
    return T(value);
}

T convertDouble<T : IFractional>(double value)
{
    return T(value);
}
```

### Scalar and Mask Associated Types

Every type conforming to `INumericShapedType` provides two associated types:

- `Scalar` is the logical element type.
- `Mask` is the result type of component-wise comparisons and classifications.

For `float`, `Scalar` is `float` and `Mask` is `bool`.
For `float3`, `Scalar` is `float` and `Mask` is `vector<bool, 3>`.
A conforming type also provides `fromScalar`, which constructs a value by splatting one `Scalar` value across its logical shape.

Most generic code does not need to name either associated type.
They become useful when an algorithm explicitly moves between an aggregate and its element domain:

```slang
T filledWith<T : INumericShapedType>(T.Scalar value)
{
    return T.fromScalar(value);
}
```

### Comparisons and Mask Reductions

`IComponentwiseEquatable` and `IComponentwiseOrdered` return the conforming type's `Mask`.
Use the free `all` and `any` functions to reduce a scalar or shaped mask to one `bool` result.

```slang
bool allLessThan<T : IComponentwiseOrdered>(T left, T right)
{
    return all(left < right);
}

void useAllLessThan()
{
    let scalarResult = allLessThan(1.0, 2.0);
    let vectorResult = allLessThan(float3(1.0), float3(2.0));
}
```

The independent `IEquatable`, `IPartiallyOrdered`, and `ITotallyOrdered` interfaces describe relations that directly return `bool`.
Their distinction matters for user-defined scalar types whose equality or order is not naturally component-wise.

### Requiring a Scalar

An interface whose name begins with `IScalar`, such as `IScalarNumeric` or `IScalarIntegerType`, constrains `Scalar` to the conforming type and `Mask` to `bool`.
Use a scalar refinement when an algorithm cannot meaningfully accept a vector, matrix, or other logical aggregate.

```slang
T chooseSmallerScalar<T : IScalarIntegerType>(T left, T right)
{
    return left < right ? left : right;
}
```

## Choosing Numeric Capabilities

The interfaces separate operations that do not have the same meaning for every useful numeric representation.
Prefer the narrowest set of capabilities that expresses what an algorithm actually uses.

### Arithmetic Capabilities

- `IAdditive` provides component-wise addition, subtraction, and the additive identity.
- `INumeric` adds component-wise multiplication and the multiplicative identity.
- `ISignedNumeric` adds negation and absolute value.
- `IIntegerType` adds integer quotient and remainder, bitwise operations, shifts, and component-wise ordering.
  `ISignedIntegerType` and `IUnsignedIntegerType` distinguish signed and unsigned integer types.
- `IFractional` adds division and reciprocal operations.
  It does not imply floating-point representation, elementary functions, ordering, or differentiability.
- `IFloatingPoint` adds operations tied to a floating-point representation, including rounding, floating-point remainder, splitting, sign copying, and classification.

The separation between `IFractional` and `IFloatingPoint` allows fixed-point, ratio, dual-number, and interval types to expose fractional arithmetic without claiming an IEEE-like representation.

### Elementary-Function Capabilities

Elementary functions are divided into independent families:

- `IExponentialFunctions` provides exponential, logarithmic, and power functions.
- `ITrigonometricFunctions` provides trigonometric functions.
- `IInverseTrigonometricFunctions` provides inverse trigonometric functions.
- `IHyperbolicFunctions` provides hyperbolic and inverse-hyperbolic functions.
- `IRootFunctions` provides square-root and reciprocal-square-root functions.

`IElementaryFunctions` is a conjunction of all five families.
These interfaces do not imply arithmetic, ordering, or differentiability.

For example, an algorithm that needs only fractional arithmetic and trigonometric functions can state exactly those capabilities:

```slang
T shiftedSine<T : IFractional & ITrigonometricFunctions>(T value, T phase)
{
    return sin(value + phase);
}
```

### Real-Ordering Capabilities

`IRealOrderingFunctions` provides component-wise minimum, maximum, and step operations, whose conventional definitions depend on a real ordering.
`IReal` is a convenience conjunction that combines fractional arithmetic, every elementary-function family, component-wise partial ordering, and `IRealOrderingFunctions`.
`IScalarReal` is its scalar refinement.

`IReal` does not imply `IFloatingPoint`.
A user-defined numeric type can provide real-valued arithmetic, elementary functions, and ordering without claiming a floating-point representation.

Conjunction aliases such as `IElementaryFunctions` and `IReal` are intended for generic constraints.
A user-defined type satisfies an alias by conforming to its constituent interfaces; it does not declare a separate nominal conformance to the alias.

## Adding Capabilities to a User-Defined Type

A custom scalar should declare `IScalarShapedType` once, then add only the operational interfaces it supports.
This shared conformance ensures that independent capabilities agree on their `Scalar` and `Mask` witnesses.

The following type exposes trigonometric functions without claiming an ordering or a particular arithmetic representation:

```slang
struct Angle
{
    float radians;
}

extension Angle : IScalarShapedType
{
    typealias Scalar = Angle;
    typealias Mask = bool;

    static Angle fromScalar(Angle value)
    {
        return value;
    }
}

extension Angle : ITrigonometricFunctions
{
    Angle sin() { return { ::sin(radians) }; }
    Angle cos() { return { ::cos(radians) }; }
    Angle tan() { return { ::tan(radians) }; }

    void sincos(out Angle sineResult, out Angle cosineResult)
    {
        sineResult = { ::sin(radians) };
        cosineResult = { ::cos(radians) };
    }
}

T sineOf<T : ITrigonometricFunctions>(T value)
{
    return sin(value);
}
```

This example intentionally conforms to only one elementary-function family.
A practical numeric type can add `IFractional`, ordering, or other elementary-function families in separate extensions as their semantics require.

## Differentiable Numeric Algorithms

The base numeric interfaces do not guarantee that calls through their requirements are differentiable.
Code that needs that guarantee imports the differentiable annex:

```slang
import slang.numerics.differentiable;

[Differentiable]
T smoothWave<T : IDifferentiableFractional & IDifferentiableTrigonometricFunctions>(T value)
{
    return sin(value) + value * value;
}
```

`slang.numerics.differentiable` re-exports `slang.numerics`, so no separate base-module import is needed.
The differentiable interfaces refine their base counterparts and re-declare operations as differentiable requirements where Slang's existing builtin types support differentiation.

Each elementary-function family has a corresponding `IDifferentiable...Functions` refinement.
`IDifferentiableElementaryFunctions` combines all of them.
`IDifferentiableFractional`, `IDifferentiableFloatingPoint`, `IDifferentiableRealOrderingFunctions`, and `IDifferentiableReal` provide the corresponding arithmetic and real-valued bundles.

Operations that are inherently discontinuous or that produce classifications remain available through the base interfaces without a differentiability guarantee.
`IDifferentiableFractional` re-declares construction from every builtin floating-point type as a differentiable requirement.
The differentiable interfaces do not guarantee differentiation through `fromScalar`, and construction from builtin integers remains a constant-producing base-interface operation rather than a differentiable requirement.

## Builtin Conformances and Current Limitations

Builtin integer and floating-point scalars and vectors receive the applicable conformances.
Floating-point matrices preserve their layout type through component-wise operations.
Cooperative vectors receive base integer, fractional, elementary-function, and ordering conformances when their element type supports the corresponding scalar capability.

The differentiable annex does not yet provide cooperative-vector conformances.
Because the modules are experimental, supported conformances and capability requirements may be refined as Slang gains experience with user-defined numeric types.
