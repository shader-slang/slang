---
layout: user-guide
---

# Writing Generic Numerical Code

> **Note:** The interfaces in this chapter belong to experimental standard modules.
> Their names, decomposition, and requirements may change in future releases.

The `slang.numerics` module provides support for writing generic code that can work with various numeric types.
Using the interfaces provided by this module, a developer can write code that:

- works with any of Slang's built-in integer or floating-point types

- works uniformly with scalars and supported shaped types, including vectors, matrices, and cooperative vectors

- defines custom numeric types (e.g., fixed-point numbers, dual numbers, etc.) that work with existing numerical code

- applies Slang's support for automatic differentiation to their generic numerical code

The rest of this chapter describes the facilities that `slang.numerics` provides, with examples of how some of the features can be applied.

## Enabling the Module

The numerics module must be imported explicitly:

```slang
import slang.numerics;
```

Because the module is experimental, compile the program with `-experimental-feature`.
Importing `slang.numerics` does not change the interfaces or overloads provided by the core module.

Note that the base `slang.numerics` module does not include support for automatic differentiation.
We discuss automatic differentiation support in more detail later, but it is important to note that applications that want to write differentiable generic numeric code should import the `slang.numerics.differentiable` module instead:

```slang
import slang.numerics.differentiable;
```

## Terminology

The `slang.numerics` module design decomposes numeric types into two broad categories:

- *shaped* types are things like `vector`, `matrix`, etc. that can be thought of as a homogeneous collection of elements,
  and where the intended semantics for mathematical operations is that they apply element-wise

- *scalar* types are the logical elements or components of shaped types.
  Mathematical operations on a scalar type are uniquely defined by the semantics of that type.

The `slang.numerics` module allows scalars to be treated as shaped types - they just have a scalar (rank 0) shape.

In most cases, the interfaces that `slang.numerics` provides are for shaped types by default, so that it is easy to write code that works cleanly for both scalars and vectors/matrices.
When a developer specifically needs operations that only make sense on scalars (e.g., a total ordering exists on integers, but Slang does not define a total ordering on *vectors* of integers), they should opt in to the scalar-specific interfaces.

> Note:
>
> A scalar numeric type may still be a composite type.
> For example, a type that implements complex or dual numbers should be considered as a scalar; element-wise semantics are not appropriate for mathematical operations like multiplication on those types.

> Note:
>
> Multiplication on shaped types is element-wise multiplication.
> This element-wise operation is consistent with the semantics of the built-in `vector` and `matrix` types, and with many other GPU and ML programming models.
>
> Element-wise multiplication may come as a surprise to developers with a more traditional mathematical background, who would expect linear-algebraic multiplication of matrices.

## A First Example

The following example shows a function that can be used with built-in integer and floating-point scalars, vectors, and supported matrices:

```slang
T squarePlusOne<T : INumeric>(T value)
{
    let one = T(1);
    return value * value + one;
}
```

The `squarePlusOne` function is generic in a single type parameter `T`, and constrains `T` to be a type that conforms to the `INumeric` interface.
Within the body of the generic, the ordinary `+` and `*` operators can be used on values of type `T`, and a `T` value can be constructed from the integer literal `1`.

> Note:
>
> `INumeric` is just one of the interfaces that `slang.numerics` provides.
> In a later section we will discuss how to pick the right interface to use as a constraint, based on the needs of your code.


Here is some code that uses `squarePlusOne`:

```slang
void useSquarePlusOne()
{
    let scalarResult = squarePlusOne(2.0);
    let vectorResult = squarePlusOne(int3(1, 2, 3));
}
```

Here we see that the single generic `squarePlusOne` function can be used with either a scalar floating-point type or an integer vector type.

An attempt to use `squarePlusOne` with a type that doesn't conform to `INumeric` results in a compile-time error:

```slang
void tryToDoMathOnBooleans()
{
    // ERROR: `bool` does not conform to `INumeric`
    let boolResult = squarePlusOne<bool>(true);
}
```

## Working with Scalar and Shaped Types

The example in the previous section used the `INumeric` interface to define a function that works for both scalars and vectors/matrices.
However, there are some operations that are not so easy to apply generically to both scalars and shaped types.

Consider the following example that tries to use comparison with `IIntegerType`, another interface provided by `slang.numerics`:

```slang
bool isPositive<T : IIntegerType>(T value)
{
    return value > T(0);
}
```

This example fails to compile as-is, because the `>` operation on a shaped value results in another shaped value.
For example, if `T` is `int3`, then the comparison results in a `bool3`, not a `bool`.

In situations where operations like comparison are involved, there are a few ways that a developer can proceed, depending on their specific situation and needs.

### Requiring a Scalar Type

The numerics module provides refined scalar-specific alternatives for many of its core interfaces.
For example, `IScalarNumeric` refines `INumeric`, and `IScalarIntegerType` refines `IIntegerType`.

By restricting the type parameter to conform to a scalar-specific interface, the preceding `isPositive` example can compile cleanly:

```slang
bool isPositive<T : IScalarIntegerType>(T value)
{
    return value > T(0);
}
```

### Adapting to Different Shapes

Rather than restricting ourselves to only scalar types for `isPositive`, we might instead decide that we want to write code that works with *any* integer type, whether scalar or shaped.
We might, for example, decide that we will consider a shaped value to be "positive" if all of its elements are positive.
In that case, we could write the function as:

```slang
bool isPositive<T : IIntegerType>(T value)
{
    return all(value > T(0));
}
```

This function can work uniformly for both vector and scalar integers.
For example, in the case where `T` is `int3`, the comparison results in a `bool3`, and the `all` function results in a single `bool` representing the logical AND of the three elements.
In the case where `T` is `int`, the comparison results in a single `bool`, and the `all` operation returns that value as-is.

We'll now drill down into how this shape-agnostic version of `isPositive` works.

#### Scalar and Mask Associated Types

All shaped types (types conforming to `INumericShapedType`) provide two associated types:

- `Scalar` is the logical element type.
- `Mask` is the result type of component-wise comparisons and classifications.

For example, the `float3` type defines its `Scalar` type as `float` and its `Mask` type as `bool3`.
For a scalar type like `float`, the `Scalar` is the same as the type itself, and the `Mask` is `bool`.

The following is what the shape-agnostic `isPositive` might look like, if broken down into more explicit steps using the associated `Scalar` and `Mask` types for `T`:

```slang
bool isPositive<T : IIntegerType>(T value)
{
    let scalarZero : T.Scalar = T.Scalar(0);
    let shapedZero : T = T.fromScalar(scalarZero);
    let shapedMask : T.Mask = value > shapedZero;
    let scalarMask : bool = all(shapedMask);
    return scalarMask;
}
```

It is often possible to write generic numeric code without using the `Scalar` or `Mask` associated types explicitly.

## Conversion from Built-In Scalar Types

All types that conform to `INumeric` support being constructed from values of any built-in integer type:

```slang
T addToNarrowInteger<T : INumeric>(T left, int8_t right)
{
    return left + T(right);
}
```

Additionally, all types that conform to `IFractional` support being constructed from values of any built-in floating-point type:

```slang
T addToDouble<T : IFractional>(T left, double right)
{
    return left + T(right);
}
```

These conversions make familiar cases like `T(1)`, `T(0.5)`, etc. available for use in generic numeric code, but they are not only usable with literals.

## Choosing the Right Numeric Interface

Different numeric types support different operations, and sometimes the same symbol will denote semantically distinct operations between types (e.g., the infix `/` operator acts quite differently between the built-in integer and floating-point types).
The numerics module provides a range of interfaces to represent these differing capabilities.

It is usually good practice for a generic algorithm to constrain its type parameters to the narrowest interface that provides the operations that the algorithm actually requires.

### Arithmetic Capabilities

The typical arithmetic operations are divided as follows:

- `IAdditive` provides addition, subtraction, and the additive identity.
- `INumeric` refines `IAdditive`, and provides multiplication and the multiplicative identity.
- `ISignedNumeric` refines `INumeric`, and provides negation and absolute value.
- `IIntegerType` refines `INumeric`, and provides integer quotient and remainder, bitwise operations, shifts, and element-wise ordering.
  `ISignedIntegerType` and `IUnsignedIntegerType` distinguish signed and unsigned integer types.
- `IFractional` refines `INumeric`, and provides division and reciprocal operations.
  It does not imply floating-point representation, elementary functions, ordering, or differentiability.
- `IFloatingPoint` refines `IFractional`, and provides operations tied to a floating-point representation, including rounding, floating-point remainder, splitting, sign copying, and classification.

The separation between `IFractional` and `IFloatingPoint` allows developers to introduce custom fixed-point, ratio, dual-number, and interval types without forcing them to provide the illusion of an IEEE-like representation.

The above interfaces all support both scalar and shaped types.
When a generic algorithm only works with scalar types, then one of the scalar-specific refinements should be used:

- `IScalarAdditive`
- `IScalarNumeric`
- `IScalarSignedNumeric`
- `IScalarIntegerType`
- `IScalarSignedIntegerType`
- `IScalarUnsignedIntegerType`
- `IScalarFractional`
- `IScalarFloatingPoint`

### Elementary-Function Capabilities

Elementary functions are divided into independent families:

- `IExponentialFunctions` provides exponential, logarithmic, and power functions.
- `ITrigonometricFunctions` provides trigonometric functions.
- `IInverseTrigonometricFunctions` provides inverse trigonometric functions.
- `IHyperbolicFunctions` provides hyperbolic and inverse-hyperbolic functions.
- `IRootFunctions` provides square-root and reciprocal-square-root functions.

`IElementaryFunctions` is a conjunction of all of the above families.

Note that the elementary-function interfaces do not on their own imply support for arithmetic, ordering, or differentiability.
When an algorithm relies on a specific combination of arithmetic operations and elementary functions, a generic type parameter should be constrained to a conjunction of the necessary operations.
For example:

```slang
T shiftedSine<T>(T value, T phase)
    where T : IFractional & ITrigonometricFunctions
{
    return sin(value + phase);
}
```

### Comparison and Ordering

Comparison and ordering operations are a key place where the difference between scalar and shaped types becomes relevant.
The numerics module provides interfaces for both element-wise shaped comparisons as well as interfaces appropriate to the scalar case.

Note that these interfaces are one place where the numerics module gives the shorter and more natural names to the *scalar* interfaces, while the shaped interfaces get the longer and more explicit names.

The following interfaces support comparison and ordering:

- `IEquatable` provides equality comparisons
- `IPartiallyOrdered` refines `IEquatable` with partial ordering (such as for IEEE floating-point values)
- `ITotallyOrdered` refines `IPartiallyOrdered` to mark types that guarantee a total order

The above interfaces define comparison operations that all return `bool` values.
These interfaces are currently used for scalar numeric types, but are appropriate for use with general (non-numeric) types that support equality comparison or ordering.

For element-wise comparisons, the numerics module provides the following interfaces:

- `IComponentwiseEquatable` provides equality comparisons
- `IComponentwiseOrdered` provides ordering comparisons

The element-wise interfaces define comparison operations that all return the associated `Mask` type for the conforming type.
Masks can be reduced to a single `bool` using the `all` and `any` functions.
For example:

```slang
bool allLessThan<T : IComponentwiseOrdered>(T left, T right)
{
    return all(left < right);
}
```

#### Real Number Ordering

The `IRealOrderingFunctions` interface provides element-wise minimum, maximum, and step-function operations whose conventional definitions depend on the ordering behavior of types such as IEEE floating-point numbers.

### The `IReal` Convenience Definition

The numerics module provides `IReal` as a convenience definition that combines fractional arithmetic, all the elementary-function interfaces, element-wise ordering, and the real-number ordering functions.
The scalar analogue of `IReal` is `IScalarReal`.

When a developer just wants to be able to write some interesting numerical code and doesn't have the time to precisely scope the operations that are required, they may use `IReal` or `IScalarReal` as a constraint on a generic type parameter to get access to a wide range of numerical operations.
For example:

```slang
R mixOfManyFunctions<R : IReal>(R x)
{
    return exp(x) + acosh(x) * (x + R(1));
}
```

The `IReal` and `IScalarReal` definitions can be useful when porting code, or during rapid prototyping, when the exact mix of operations needed is not precisely known.
It is typically best to declare a narrower constraint on a generic type parameter once the requirements are stable and understood.

## Adding Custom Numeric Types

The interfaces in the numerics module are intended to support not only the built-in scalar and shaped numeric types, but also user-defined numeric types.

As an example, consider a simple user-defined type for complex numbers:

```slang
struct MyComplex
{
    float x;
    float y;
}
```

The first step to making the `MyComplex` type usable with generic numerics code is to make it conform to the `IScalarShapedType` interface:

```slang
extension MyComplex : IScalarShapedType
{
    typealias Scalar = This;
    typealias Mask = bool;

    static This fromScalar(This value)
    {
        return value;
    }
}
```

Next, we can use additional `extension`s to make the user-defined type conform to the interfaces for whatever operations we want to support.
For example:

```slang
extension MyComplex : IScalarAdditive
{
    static This zero()
    {
        return { 0.0, 0.0 };
    }

    MyComplex plus(MyComplex that)
    {
        return { this.x + that.x, this.y + that.y };
    }

    MyComplex minus(MyComplex that)
    {
        return { this.x - that.x, this.y - that.y };
    }
}
```

Once `MyComplex` is declared to conform to `IScalarAdditive`, the ordinary infix `+` and `-` operators will be usable with values of type `MyComplex`.
Any user-defined generic functions with a type parameter constrained to `IAdditive` or `IScalarAdditive` will also be usable with the `MyComplex` type.

A practical implementation of a new numeric type would likely want to cover all the applicable interfaces for arithmetic operations, elementary functions, etc.
Conformances for operation families can be added one at a time, via additional `extension`s, rather than having to be written all at once.

## Differentiable Numeric Algorithms

The numeric interfaces discussed so far define various mathematical operations, but do not guarantee that Slang's automatic differentiation can differentiate through calls to those operations.
When writing generic numerical code that must also be differentiable, developers should import the `slang.numerics.differentiable` module and use the more refined differentiable interfaces there.
For example:

```slang
import slang.numerics.differentiable;

[Differentiable]
T smoothWave<T>(T value)
    where T : IDifferentiableFractional
              & IDifferentiableTrigonometricFunctions
{
    return sin(value) + value * value;
}
```

The `slang.numerics.differentiable` module re-exports all of the definitions from `slang.numerics`, so a separate import of the base module is not needed.

The differentiable numerics module provides various `IDifferentiable...` counterparts to interfaces from the base numerics module:

- `IDifferentiableNumericShapedType`
- `IDifferentiableFractional`
- `IDifferentiableFloatingPoint`
- `IDifferentiableRealOrderingFunctions`
- `IDifferentiableExponentialFunctions`
- `IDifferentiableTrigonometricFunctions`
- `IDifferentiableInverseTrigonometricFunctions`
- `IDifferentiableHyperbolicFunctions`
- `IDifferentiableRootFunctions`
- `IDifferentiableElementaryFunctions`
- `IDifferentiableReal`

Operations that are inherently discontinuous or that produce classifications remain available through the base interfaces, without a differentiability guarantee.

Conversion to an `IDifferentiableFractional` type from scalar floating-point values, whether through construction with a value of a built-in floating-point type or through the `fromScalar` operation, preserves differentiability when the scalar value is itself differentiable.

## Built-In Conformances and Current Limitations

The numerics module defines conformances to the appropriate interfaces for:

- the built-in integer and floating-point scalar types
- vectors of built-in scalar types
- matrices of built-in floating-point types
- cooperative vectors of built-in integer or floating-point types

Cooperative vectors of built-in integer types conform to the applicable integer interfaces.
Cooperative vectors of built-in floating-point types conform to `IFractional`, the elementary-function interfaces, `IComponentwiseOrdered`, and `IRealOrderingFunctions`, and therefore satisfy the `IReal` convenience definition.
Importing `slang.numerics.differentiable` adds the corresponding differentiable conformances, so floating-point cooperative vectors also satisfy `IDifferentiableReal`.

Known limitations include:

- Matrices of built-in integer types do not currently conform to the numeric interfaces.
- Cooperative vectors of built-in floating-point types satisfy `IReal`, but do not currently conform to `IFloatingPoint`.
- Cooperative matrices do not currently conform to the numeric interfaces.

Because the numerics modules are experimental, the set of supported conformances is expected to change as the design and implementation of the module evolves.
