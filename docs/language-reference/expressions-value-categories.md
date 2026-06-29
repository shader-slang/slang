Value Categories
================

Slang has two primary value categories. In principle:

- An _L-value_ has an underlying mutable storage. Hence, an L-value can be on the left side of an assignment,
  and it can be used as an argument to an `out`/`inout` function parameter.
- An _R-value_ is a temporary value. It can only be used in contexts where a pure value is adequate. It cannot
  be used as an argument to an `out`/`inout` function parameter. An R-value is typically a result of a
  computation, a value passed as a copy, or a result of a function invocation.

The following are the L-value expressions:

1. Access to a mutable variable
2. Access to an `out`/`inout`/`__ref` parameter
3. Access to an `in`-parameter that has been declared using the traditional variable declaration (C-style)
   - Note: the argument is still passed by value copy
4. Pointer dereference when the pointer access type is `Access.ReadWrite`.
5. Access to a reference
6. Access to a property that has a setter accessor and when the base is an L-value
7. Applying a subscript operator when both:
   - The base is an L-value
   - The subscript operator is a built-in operator OR the subscript operator is user-defined with a setter
     accessor
8. Access to `this` in a mutating context
9. Return value for a non-copyable return type
10. Access to a member if and only if the base is an L-value
11. Swizzle expression when both:
   - The base is an L-value
   - The swizzle expression has no duplicate components
12. Cast of an L-value object to a conforming type
13. Access to an interface-typed mutable object (dynamic dispatch)
14. Implicit casts that preserve L-value (`inout`/`out` parameters)

