Basic Interfaces
================

The Slang standard library is in need of basic interfaces that allow generic code to be written that abstracts over built-in types.
This document sketches what the relevant interfaces and their operations might be.

Status
------

In discussion.

Background
----------

One of the first things that a user who comes from C++ might try to do with generics in Slang is write an operation that works across `float`s, `double`s, and `half`s:

```
T horizontalSum<T>( vector<T,4> v )
{
    return v.x + v.y + v.z + v.w;
}
```

A function like `horizontalSum` does not compile because without a constraint on the type parameter `T`, the compiler has no reason to assume that `T` supports the `+` operator.
A new user is often stymied at this point, because no appropriate `interface` seems to exist, and there does not appear to be a way to *define* an appropriate interface.

As a user gets more experienced with Slang, they may learn how to use `extension`s to define something nearly suitable:

```
interface IMyAddable { This add(This rhs); }

extension float : IMyAddable { float add(float rhs) { return this + rhs; } }
// ...

T horizontalSum<T : IMyAddable>( vector<T,4> v )
{
    return v.x.add(v.y).add(v.z).add(v.w);
}
```

While that approach works (or should work), it requires a user to know how to use `extension`s and the `This` type, which are complicated even for experienced users. The resulting code is also less readable because it uses `.add(...)` instead of the ordinary `+` operator.

Many more users end up finding out about the `__BuiltinFloatingPointType` interface, and write something like:

```
T horizontalSum<T : __BuiltinFloatingPointType>( vector<T,4> v )
{
    return v.x + v.y + v.z + v.w;
}
```

This alternative is much more palatable to users, but it results in them using a double-underscored interface (which we consider to mean "implementation details that are subject to change"). Users often get tripped up when they find out that certain operations that make sense to be available through `__BuiltinFloatingPointType` are not available (because those operations were not needed in the definition of the stdlib, which is what the `__` interfaces were created to support).

Related Work
------------

There are several languages that have constructs similar to our `interface`s, and which provide built-in interfaces for simple math operations that are suitable for use with the built-in types provided by the language.

Existing solutions can be broadly categorized based on whether their built-in interfaces are related to semantic/mathematical structures, or are purely about specific classes of operators.

Haskell and Swift are both examples of languages where the built-in interfaces are intended to be semantic. Haskell provides type classes such as `Additive`, `Ring`, `Algebraic`, `RealTranscendental`, etc.
Swift is similar (although it provides a less complete hierarchy of algebraic structures than Haskell), but also includes more detail amount machine number representations, so that it has `BinaryFloatingPoint`, `FixedWidthInteger`, etc.

Rust is in the other camp, where it has a built-in interface to correspond to each of its overloadable operators. The `Add` and `Sub` traits allow the built-in `+` and `-` operators to be overloaded for a user-defined type, but impose no implicit or explicit semantic expectations on those operations.

It may help to describe a concrete example of how the difference between the two camps affects design. The Rust `Add` trait is implemented by the left-hand-side type, and does not constrain the right-hand side or result type of an addition. A Rust programmer may implement `Add` for a type `X` so that `x + ...` expects a right-hand-side operand of some other type `Y` and produces a result of yet *another* type `Z`. Knowing that `X` supports the `Add` trait does *not* mean that it is possible to take the sum of a list of `X`s, because there is no guarantee that `x0 + x1` is valid, or that `X` has a logical "zero" value that could be used as the sum of an empty list.

In contrast, in Swift a type `X` that conforms to `AdditiveArithmetic` must provide a `+` operation that takes two `X` values and yields an `X`. It also requires that `X` provide a `static` property `zero` of type `X`, to represent its zero value. As a result, it is possible to write a generic function in switch that can compute the sum of a list of `T` values, provided `T` conforms to `AdditiveArithmetic`.

Proposed Approach
-----------------

Slang supports operators as ordinary overloadable functions, so the rationale behind the Rust operator traits does not seem to apply. We propose to implement a modest hierarchy of numeric interfaces in the style of Haskell/Swift.

### Changes to Operator Lookup

Currently, when Slang encounters an operator invocation like `a + b`, it treats this as more or less equivalent to a function call `+( a, b )`. The compiler looks up `+` in the current lexical environment, and then applies overload resolution to the result of lookup.

We propose that the rules in that case should be changed so that lookup *also* perform lookup of the operator (`+` in this case) in the context of the static types of `a` and `b`. That change would in theory allow "operator overloads" to be defined as `static` functions within a type they apply to (whether on the left-hand or right-hand side). As a consequence, such a change would also mean that `interface`s could conveniently include operator overloads as requirements.

### IAdditive

The `IAdditive` interface is for types where addition, subtraction, and zero have meaning.

```
interface IAdditive
{
    // The zero value for this type
    static property zero : This { get; }

    // Add two values of this type
    static func +(left: This, right: This) -> This;

    // Subtract two values of this type
    static func -(left: This, right: This) -> This;
}
```

### INumeric

The `INumeric` interface is for types that are more properly number-like.
Note that this interface does not define division, because the division operations on integers and floating-point numbers are sufficiently different in semantics.

```
interface INumeric : IAdditive
{
    // Initialize from an integer
    __init< T : IInteger >( T value );

    // Multiply two values of this type
    static func *(left: This, right: This) -> This;
}
```

### ISignedNumeric

Only signed numbers logically support negation (although we all know it also gets applied to unsigned numbers, where it has meaningful and use semantics).

```
interface ISignedNumeric : INumeric
{
    // Negate a value of this type
    static prefix func -(value: This) -> This;
}
```


### IInteger

The `IInteger` interface codifies the basic things that a generic wants to be able to access for any integer type.

```
interface IInteger : INumeric
{
    // Smallest representable value
    static property minValue : This { get; }

    // Largest representable value
    static property maxValue : This { get; }

    // Initialize from a floating-point value
    // (what rounding mode? round-to-nearest-even?)
    __init< T : IFloatingPoint >( T value );


    // Integer quotient
    static func /(left: This, right: This) -> This;

    // Integer remainder (or is it modulus? or is it undefined which?)
    static func %(left: This, right: This) -> This;
}
```
### IUnsignedInteger

```
interface IUnsignedInteger : IInteger
{

}
```

### ISignedInteger

The main interesting thing we'd want from a signed integer type is to be able to convert it to the same-size unsigned integer type.

```
interface ISignedInteger : IInteger, ISignedNumeric
{
    // Equivalent unsigned type (can always hold magnitude)
    associatedtype Unsigned : IUnsignedInteger;

    // Get the magnitude of this value (may not be representable
    // as `This` type, if it is `minValue`)
    property magnitude : Unsigned { get; };
}
```

### IFloatingPoint

The `IFloatingPoint` interface provides the minimum of what users expect a floating-point type to support.
It includes the ability to check for special values (not-a-number, infinities), as well as the value of various standard constants.

```
interface IFloatingPoint : INumeric, ISignedNumeric
{
    property isFinite : bool { get; }
    property isInfinite : bool { get; }
    property isNaN : bool { get; }
    property isNormal : bool { get; }
    property isDenormal : bool { get; }

    // TODO: breaking into magnitude/exponent

    static property infinity : This { get; }
    static property nan : This { get; }
    static property pi : This { get; }

    // TODO: min/max finite values, smallest non-zero value, etc.

    // Initialize from another floating-point value.
    __init< T : IFloatingPoint >( T value );

    // Floating-point division
    static func /(left: This, right: This) -> This;
}
```

### ISpecialFunctions

The `ISpecialFunctions` interface is for floating-point types that also have full support for the standard suite of special functions provided by something like `<math.h>`.
It is pulled out as a distinct interface from `IFloatingPoint` because many platforms support floating-point types like `double` without also having full support for special functions on those types.

```
interface ISpecialFunctions : IFloatingPoint
{
    static This cos(This value);
    static This sin(This value);
    // TODO: fill this out
}
```

Questions
---------

### Should these all be `IBuiltin*`? Should we have separate interfaces for built-in and user types?

The main reason for the current `__Builtin` interfaces is that it allows us to define built-in functions that are generic over those interfaces, but which map to a single instruction in the Slang IR. The relevant operations are not currently defined as

### What should the naming convention be for `interface`s in Slang?

These would be the first `interface`s officially exposed by the standard library.
While most of our existing code written in Slang uses an `I` prefix as the naming convention for `interface`s (e.g., `IThing`), we have never really discussed that choice in detail.
Whatever we decide to expose for this stuff is likely to become the de facto convention for Slang code.

The `I` prefix is precedented in COM and C#/.net/CLR, which are likely to be familiar to many devleopers using Slang.
Because of COM, it is also the convention used in the C++ API headers for Slang and GFX.

The Rust/Swift languages do not distinguish between traits/protocols and other types.
This choice is intentional, and it might be good to understand the motivation behind it.
At least one potential benefit to not distinguishing such types is that beginning programmers can write code that is "more generic" than they might otherwise write.

Alternatives Considered
-----------------------

One important alternative is to follow the precedent of Rust and avoid basing these interfaces on semantic structures.
That choice is important in Rust in part because there is no way for a type to support an operator other than by implementing the built-in operator traits.
If the operator traits had prescriptive semantics, they might cause problems for types that want to support the operators but cannot fit within the semantic constraints.
In contrast, Slang allows operator overloads to be defined independent of interfaces (they are orthogonal features), so there is no risk of developers being "locked in" by our attempts to provide richer interfaces.

Conversely, one could worry that our interfaces do not provide *enough* semantics. We may find that users need additional interfaces that sit "in between" these ones, or that carve up the same operations into smaller units.
This proposal contends that we need to have *something* in this space, and that it doesn't make sense to try to get these interfaces 100% perfect until we've had some lived experience with them.
Fortunately, the Slang language is not yet at a point of trying to guarantee perfect source stability of these interfaces, nor anything like strong binary compatibility guarantees.
If we make mistakes here, we have time to fix them.

