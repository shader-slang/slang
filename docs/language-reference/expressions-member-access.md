> TODO

Member Expression
-----------------

**Grammar:**

> *`namespace-identifier`* (**`'.'`** | **`'::'`**) *`member-identifier`*
>
> *`type-expr`* (**`'.'`** | **`'::'`**) *`member-identifier`*
>
> *`value-expr`* **`'.'`** *`member-identifier`*

A _member access expression_ selects a member of a namespace, type, or a value expression.

If the left-hand-side is a namespace identifier, the member access expression is a qualified lookup in that
namespace. The expression type may be an arbitrary namespace member including a value, a type, or a
namespace. See [name lookup](basics-name-lookup.md) for details.

If the left-hand-side is a type expression, the member access expression is a static member lookup within that
type. See [enumerations](types-enum.md), [structs](types-struct.md), and [name lookup](basics-name-lookup.md)
for details.

If the left-hand-side is a value, then:

1. If the value is a scalar, vector, matrix, or tuple value, the member access expression is a swizzle
   expression. See *Swizzle Expressions* below.

2. If the value type is a [struct](types-struct.md), then the:

   1. If *`member-identifier`* names a member (a field or a function), then the expression is a value
      expression for that member, matching the value category.

   2. If *`member-identifier`* names a property, then the expression is translated as a `get` or `set`
      accessor of that property, depending on whether the expression reads or assigns that property.


TODO

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

When the base expression of a member expression is of a matrix type, then a
member expression with HLSL-style matrix swizzle syntax is a _matrix swizzle
expression_. Supported forms include zero-based `_mij` components and
one-based shorthand components such as `_41`. Multiple components can be
combined, for example `m._41_32`.

A matrix swizzle expression is an l-value if the base expression was an l-value
and the swizzle does not select duplicate matrix elements. Constant-indexed
subscripts of matrix swizzle l-values are also l-values.

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
