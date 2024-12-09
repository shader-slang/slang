# SP #015: Abstract Algebra Interface Hierarchy

This proposal introduces a hierarchy of algebraic structure interfaces in
Slang, providing a foundation for principled generic mathematical
operations.

## Status

Status: Design Review

Implementation: N/A

Author: Ellie Hermaszewska

Reviewer: TBD

## Background

Currently, Slang lacks a formal way to express algebraic properties of types.
While basic arithmetic operations are supported, there's no standard vocabulary
to express a type's conformance to algebraic laws. This is particularly
relevant for graphics programming where we frequently work with a variety of
mathematical objects, each fulfilling different properties described by
abstract algebra, for example:

- Vector spaces
- Quaternions
- Dual numbers
- Matrices
- Complex numbers

Additionally, many types support only a subset of arithmetic operations. For
example:

- Strings support concatenation (forming a monoid) but not subtraction
- Colors support addition and scalar multiplication but not division
- Matrices support multiplication but aren't always invertible
- Non-empty containers support concatenation (forming a semigroup) but lack an
  identity element

A single catch-all "Arithmetic" interface would be insufficient as it would
force types to implement operations that don't make mathematical sense

A formal interface hierarchy allows us to:

- Write generic code that works across multiple numeric types
- Clearly document mathematical properties
- Avoid forcing inappropriate operations on types

## Related Work

- Haskell: Extensive typeclass hierarchy (Semigroup, Monoid)
  - Haskell does have a catch-all `Num` class, which is largely considered a
    misfeature, and several third-party packages exist to rectify this with a
    more nuanced abstract algebra hierarchy
- PureScript: Similar to Haskell but with stricter adherence to category theory
  principles
- Rust: Traits for arithmetic operations (Add, Mul)
- Scala: Abstract algebra typeclasses in libraries like Spire

## Proposed Approach

Implement the following interfaces, replacing in part `__BuiltinArithmeticType`

```slang
// Semigroup represents types with an associative addition operation
// Examples: Non-empty arrays (concatenation), Colors (blending)
interface Semigroup
{
    T operator+(T a, T b);  // Associative addition
};

// Monoid adds an identity element to Semigroup
// Examples: float3(0), identity matrix, zero quaternion
interface Monoid : Semigroup
{
    static T zero();  // Additive identity
};

// Group adds inverse elements to Monoid
// Examples: Vectors under addition, Matrices under addition
interface Group : Monoid
{
    T operator-();  // Additive inverse
};

// CommutativeGroup ensures addition order doesn't matter
// Examples: Vectors, Colors (in linear space)
interface CommutativeGroup : Group
{
    // Enforces a + b == b + a
};

// Semiring adds multiplication with identity to Monoid
// Examples: Matrices, Quaternions
interface Semiring : Monoid
{
    T operator*(T a, T b);
    static T one();
};

// Ring combines CommutativeGroup with multiplication
// Examples: Dual numbers, Complex numbers
interface Ring : CommutativeGroup
{
    T operator*(T a, T b);
    static T one();
};

// CommutativeRing ensures multiplication order doesn't matter
// Examples: Complex numbers, Dual numbers
interface CommutativeRing : Ring
{
    // Enforces a * b == b * a
};

// EuclideanRing adds division with remainder
// Examples: Integers for texture coordinates
interface EuclideanRing : CommutativeRing
{
    T operator/(T a, T b);
    T operator%(T a, T b);
    uint norm(T a);
};

// DivisionRing adds multiplicative inverses
// Examples: Quaternions
interface DivisionRing : Ring
{
    T recip();  // Multiplicative inverse (named to avoid confusion with matrix inverse)
};

// Field combines CommutativeRing with DivisionRing
// Examples: Real numbers, Complex numbers
interface Field : CommutativeRing, DivisionRing
{
};
```

## Implementation notes

Part of the scope of this work is to determine how close to this design we can
achieve and not break backwards compatability. Or, what elements of backwards
compatability we are willing to compromise on if any.

## Interface Rationale

Each interface has specific types that satisfy it but not more specialized
interfaces. A selection of non-normative examples.

- Semigroup: Non-empty arrays/buffers under concatenation (no identity element)
- Monoid: Strings under concatenation (no inverse operation)
- Group: Square matrices under addition (multiplication isn't distributive)
- CommutativeGroup: Vectors under addition (no multiplication)
- Semiring: Boolean algebra (no additive inverses)
- Ring: Square matrices under standard operations (multiplication isn't
  commutative)
- CommutativeRing: Dual numbers (no general multiplicative inverse)
- EuclideanRing: Integers (no multiplicative inverse)
- DivisionRing: Quaternions (multiplication isn't commutative)
- Field: Complex numbers, Real numbers (satisfy all field axioms)

## Floating Point Considerations

The algebraic laws described must account for floating point arithmetic
limitations:

- Associativity is not exact: `(a + b) + c ≠ a + (b + c)` for some values
- Distributivity has rounding errors: `a * (b + c) ≠ (a * b) + (a * c)`

Therefore, implementations should:

- Consider operations "lawful" if they're within acceptable epsilon
- Document precision guarantees
- Consider NaN and Inf as special cases

## Operator Precedence

All operators maintain their existing precedence rules. For example:

```slang
a + b * c  // equivalent to a + (b * c)
```

## Literal Conversion Interfaces

```slang
// Types that can be constructed from integer literals
interface FromInt
{
    static T fromInt(int value);
};

// Types that can be constructed from floating point literals
interface FromFloat
{
    static T fromFloat(float value);
};
```

Design choices still TBD:

- Handling of integer literals that exceed the target type's range
- Treatment of unsigned integer literals
- Conversion of double precision literals to single precision types
- Error reporting mechanisms for out-of-range values
- Whether to provide separate interfaces for different integer types (int32,
  uint32, etc.)

## Generic Programming

This hierarchy enables generic algorithms that work with any type satisfying
specific algebraic properties:

```slang
// Generic linear interpolation for any Field
T lerp<T : Field>(T a, T b, T t)
{
    return a * (T.one() - t) + b * t;
}

// Generic accumulation for any Monoid
T sum<T : Monoid>(T[] elements)
{
    T result = T.zero();
    for(T elem in elements)
        result = result + elem;
    return result;
}

// Generic matrix multiplication
matrix<T, N, P>; mul<T : Ring, let N : int, let M : int, let P : int>(
    a : matrix<T, N, M>,
    b : matrix<T, M, P>
)
{
    // Implementation using only Ring operations
}


// Generic geometric transforms
Transform<T : Ring> compose<T>(Transform<T> a, Transform<T> b)
{
    // Implementation using Ring operations
}
```

## Alternatives Considered

### More nuanced hierarchy

We could go a little further an introduce structures such as a `Magma`, which
could be used to represent non-associative binary operations without an
identity element. This could be used for example for some color mixing, however
this has a major downside in that it defies programmer intuition that the `+`
operator is associative.

### Operator Overloading Only

We could rely solely on operator overloading without formal interfaces.
Rejected because:

- Lawless, hard to reason about code
- Less clear documentation of mathematical properties
- Harder to write generic code with specific algebraic requirements

### Leave this to a third party library

These operations describe very fundamental aspects of the language, leaving
this to a third party library would risk incompatible interfaces arising. We
also have many higher level constructs already in the standard library which
will need to built upon some algebraic hierarchy.

### Alternative: Heterogeneous Operation Types

Similar to Rust's approach, we could allow operations to return different types than their inputs:

```slang
interface Add<RHS>
{
    associatedtype Output;
    Output operator+(This, RHS);
}
```

This would enable operations like:

```slang
extension matrix<float, 3, 3> : Mul<float3>
{
    associatedtype Output = float3;
    Output operator*(This a, Vector3 b) { /* ... */ }
}

extension float : Add<float>
{
    associatedtype Output = float3;
    Output operator+(This a, float b) { /* ... */ }
}
```

This comes with some downsides however:

- Non-injective type families (where multiple input type combinations could
  produce the same output type) may lead to worse type inference.
- This flexibility, while powerful, introduces potential confusion about
  operation semantics
- Most mathematical structures in abstract algebra assume operations are closed
- The rare cases where heterogeneous operations are needed can be served
  by explicit conversion functions or dedicated methods. This can also be
  served by implicit conversions.

The benefits of simpler type inference and clearer algebraic semantics outweigh
the flexibility of heterogeneous operations.

## Future Extensions

Several potential extensions could enhance this algebraic hierarchy:

- Vector Space Interface

  - Add dedicated interfaces for vector spaces over fields
  - Include operations for scalar multiplication and inner products
  - Support for normed vector spaces

- Ordered Algebraic Structures

  - Introduce interfaces for ordered rings and fields
  - Support comparison operators (<, >, <=, >=)
  - Enable generic sorting and optimization algorithms

- Lattice Structures

  - Add interfaces for lattices and boolean algebras
  - Support min/max operations
  - Interval arithmetic

- Module Interface

  - Support for modules over rings
  - Generalize vector space concepts
  - Enable more generic linear algebra operations

- Error Handling Extensions
  - Refined error types for algebraic operations
  - Optional bounds checking interfaces
  - Overflow behavior specifications
