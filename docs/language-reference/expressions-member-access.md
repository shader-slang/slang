> TODO

Member Expression
-----------------

A _member expression_ consists of a base expression followed by a dot (`.`) and an identifier naming a member to be accessed:

```hlsl
base.m
```

When `base` is a structure type, this expression looks up the field or other member named by `m`.
Just as for an identifier expression, the result of a member expression may be overloaded, and might be disambiguated based on how it is used.

A member expression is an l-value if the base expression is an l-value and the member it refers to is mutable.

### Implicit Dereference

If the base expression of a member reference is a _pointer-like type_ such as `ConstantBuffer<T>`, then a member reference expression will implicitly dereference the base expression to refer to the pointed-to value (e.g., in the case of `ConstantBuffer<T>` this is the buffer contents of type `T`).

### Vector Swizzles

When the base expression of a member expression is of a vector type `vector<T,N>` then a member expression is a _vector swizzle expression_.
The member name must conform to these constraints:

* The member name must comprise between one and four ASCII characters
* The characters must be come either from the set (`x`, `y`, `z`, `w`) or (`r`, `g`, `b`, `a`), corresponding to element indics of (0, 1, 2, 3)
* The element index corresponding to each character must be less than `N`

If the member name of a swizzle consists of a single character, then the expression has type `T` and is equivalent to a subscript expression with the corresponding element index.

If the member name of a swizzle consists of `M` characters, then the result is a `vector<T,M>` built from the elements of the base vector with the corresponding indices.

A vector swizzle expression is an l-value if the base expression was an l-value and the list of indices corresponding to the characters of the member name contains no duplicates.

### Matrix Swizzles

> Note: The Slang implementation currently doesn't support matrix swizzles.

### Static Member Expressions

When the base expression of a member expression is a type instead of a value, the result is a _static member expression_.
A static member expression can refer to a static field or static method of a structure type.
A static member expression can also refer to a case of an enumeration type.

A static member expression (but not a member expression in general) may use the token `::` instead of `.` to separate the base and member name:

```hlsl
// These are equivalent
Color.Red
Color::Red
```

### Subscript Operator

TODO: `[]`

### Member Access Operator

TODO: `.`
