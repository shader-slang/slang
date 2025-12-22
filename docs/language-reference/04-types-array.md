Array Types
-----------

An _array type_ is either a statically-sized or dynamically-sized array type.

A known-size array type is written `T[N]` where `T` is a type and `N` is a specialization-time constant integer.
This type represents an array of exactly `N` values of type `T`.

An unknown-size array type is written `T[]` where `T` is a type.
This type represents an array of some fixed, but statically unknown, size.

> Note: Unlike in C and C++, arrays in Slang are always value types, meaning that assignment and parameter passing of arrays copies their elements.

### Declaration Syntax

For variable and parameter declarations using traditional syntax, a variable of array type may be declared by using the element type `T` as a type specifier (before the variable name) and the `[N]` to specify the element count after the variable name:

```hlsl
int a[10];
```

Alternatively, the array type itself may be used as the type specifier:

```hlsl
int[10] a;
```

When using the `var` or `let` keyword to declare a variable, the array type must not be split:

```hlsl
var a : int[10];
```

> Note: when declaring arrays of arrays (often thought of as "multidimensional arrays") a programmer must be careful about the difference between the two declaration syntaxes.
> The following two declarations are equivalent:
>
> ```hlsl
> int[3][5] a;
> int a[5][3];
> ```
>
> In each case, `a` is a five-element array of three-element arrays of `int`s.
> However, one declaration orders the element counts as `[3][5]` and the other as `[5][3]`.

### Element Count Inference

When a variable is declared with an unknown-size array type, and also includes an initial-value expression:

```hlsl
int a[] = { 0xA, 0xB, 0xC, 0xD };
```

The compiler will attempt to infer an element count based on the type and/or structure of the initial-value expression.
In the above case, the compiler will infer an element count of 4 from the structure of the initializer-list expression.
Thus the preceding declaration is equivalent to:

```hlsl
int a[4] = { 0xA, 0xB, 0xC, 0xD };
```

A variable declared in this fashion semantically has a known-size array type and not an unknown-size array type; the use of an unknown-size array type for the declaration is just a convenience feature.

### Standard Layout

The _stride_ of a type is the smallest multiple of its alignment not less than its size.

Using the standard layout for an array type `T[]` or `T[N]`:

* The _element stride_ of the array type is the stride of its element type `T`
* Element `i` of the array starts at an offset that is `i` times the element stride of the array
* The alignment of the array type is the alignment of `T`
* The size of an unknown-size array type is unknown
* The size of a known-size array with zero elements is zero
* The size of a known-size array with a nonzero number `N` of elements is the size of `T` plus `N - 1` times the element stride of the array

### C-Style Layout

The C-style layout of an array type differs from the standard layout in that the size of a known-size array with a nonzero number `N` of elements is `N` times the element stride of the array.

### D3D Constant Buffer Layout

The D3D constant buffer layout of an array differs from the standard layout in that the element stride of the array is set to the smallest multiple of the alignment of `T` that is not less than the stride of `T`

