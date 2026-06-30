# Value Categories

Slang has two primary value categories:

- An _L-value_ has underlying mutable storage. Hence, an L-value can be on the left side of an assignment,
  and it can be used as an argument to an `out`/`inout` function parameter.
- An _R-value_ is a temporary or otherwise immutable value. It can only be used in contexts where a pure value
  is adequate. It cannot be used as an argument to an `out`/`inout` function parameter. An R-value is
  typically a result of a computation, a value passed as a copy, or a result of a function invocation.

The following are the L-value expressions:

1. Access to a mutable variable
2. Access to an `out`/`inout`/`__ref` parameter
3. Access to an `in` parameter that has been declared using the traditional variable declaration (C-style)
   - Note: the argument is still passed by value copy
4. Pointer dereference when the pointer access type is `Access.ReadWrite`
5. Access to a reference
6. Access to a property when it has a setter accessor and the base is an L-value
7. Applying a subscript operator when both:
   - The base is an L-value
   - The subscript operator is a built-in operator OR the subscript operator is user-defined with a setter
     accessor
8. Access to `this` in a mutating context
9. Return value for a non-copyable return type
10. Access to a member when the base is an L-value
11. Swizzle expression when both:
    - The base is an L-value
    - The swizzle expression has no duplicate components
12. Cast of an L-value object to a conforming type
13. Access to an interface-typed mutable object (dynamic dispatch)
14. Implicit casts that preserve L-value (`inout`/`out` parameters)

Other expressions are R-value expressions.

Slang expressions are also categorized as addressable and non-addressable:

- An _addressable_ value has underlying storage that may be mutable, read-only, or immutable. Its address
  may be requested with `__getAddress()` on targets that support pointers.
- A _non-addressable_ value does not have underlying storage, and in that sense, it is a true temporary
  value.

> 📝 **Remark:** Pointer support in Slang is currently experimental. The details are subject to change.

## Where L-values Are Required

L-values are required in the following places:

- Argument to an `out`/`inout`/`__ref` parameter
- Left-side-operand of an assignment operator, including compound assignment operators
- Operand to pre-increment, pre-decrement, post-increment, and post-decrement operators
- Object receiving a `[mutating]` or `[ref]` member function call
- Object receiving a property setter call or a user-defined subscript operator setter call, unless the setter
  has been explicitly marked as `[nonmutating]`.
