Vector Types
------------

A vector type is written as `vector<T, N>` and represents an `N`-element vector with elements of type `T`.
The _element type_ `T` must be one of the built-in scalar types, and the _element count_ `N` must be a specialization-time constant integer.
The element count must be between 2 and 4, inclusive.

A vector type allows subscripting of its elements like an array, but also supports element-wise arithmetic on its elements.
_Element-wise arithmetic_ means mapping unary and binary operators over the elements of a vector to produce a vector of results:

```hlsl
vector<int,4> a = { 1, 2, 30, 40 };
vector<int,4> b = { 10, 20, 3, 4 };

-a; // yields { -1, -2, -30, -40 }
a + b; // yields { 11, 22, 33, 44 }
b / a; // yields { 10, 10, 0, 0 }
a > b; // yields { false, false, true, true }
```

A vector type is laid out in memory as `N` contiguous values of type `T` with no padding.
The alignment of a vector type may vary by target platforms.
The alignment of `vector<T,N>` will be at least the alignment of `T` and may be at most `N` times the alignment of `T`.

As a convenience, Slang defines built-in type aliases for vectors of the built-in scalar types.
E.g., declarations equivalent to the following are provided by the Slang core module:

```hlsl
typealias float4 = vector<float, 4>;
typealias int8_t3 = vector<int8_t, 3>;
```

### Legacy Syntax

For compatibility with older codebases, the generic `vector` type includes default values for `T` and `N`, being declared as:

```hlsl
struct vector<T = float, let N : int = 4> { ... }
```

This means that the bare name `vector` may be used as a type equivalent to `float4`:

```hlsl
// All of these variables have the same type
vector a;
float4 b;
vector<float> c;
vector<float, 4> d;
```

Matrix Types
------------

A matrix type is written as `matrix<T, R, C>` and represents a matrix of `R` rows and `C` columns, with elements of type `T`.
The element type `T` must be one of the built-in scalar types.
The _row count_ `R` and _column count_ `C` must be specialization-time constant integers.
The row count and column count must each be between 2 and 4, respectively.

A matrix type allows subscripting of its rows, similar to an `R`-element array of `vector<T,C>` elements.
A matrix type also supports element-wise arithmetic.

Matrix types support both _row-major_ and _column-major_ memory layout.
Implementations may support command-line flags or API options to control the default layout to use for matrices.

> Note: Slang currently does *not* support the HLSL `row_major` and `column_major` modifiers to set the layout used for specific declarations.

Under row-major layout, a matrix is laid out in memory equivalently to an `R`-element array of `vector<T,C>` elements.

Under column-major layout, a matrix is laid out in memory equivalent to the row-major layout of its transpose.
This means it will be laid out equivalently to a `C`-element array of `vector<T,R>` elements.

As a convenience, Slang defines built-in type aliases for matrices of the built-in scalar types.
E.g., declarations equivalent to the following are provided by the Slang core module:

```hlsl
typealias float3x4 = matrix<float, 3, 4>;
typealias int64_t4x2 = matrix<int64_t, 4, 2>;
```

> Note: For programmers using OpenGL or Vulkan as their graphics API, and/or who are used to the GLSL language,
> it is important to recognize that the equivalent of a GLSL `mat3x4` is a Slang `float3x4`.
> This is despite the fact that GLSL defines a `mat3x4` as having 3 *columns* and 4 *rows*, while a Slang `float3x4` is defined as having 3 rows and 4 columns.
> This convention means that wherever Slang refers to "rows" or "columns" of a matrix, the equivalent terms in the GLSL, SPIR-V, OpenGL, and Vulkan specifications are "column" and "row" respectively (*including* in the compound terms of "row-major" and "column-major")
> While it may seem that this choice of convention is confusing, it is necessary to ensure that subscripting with `[]` can be efficiently implemented on all target platforms.
> This decision in the Slang language is consistent with the compilation of HLSL to SPIR-V performed by other compilers.


### Matrix Operations

Matrix types support several operations:

* Element-wise operations (addition, subtraction, multiplication) using the standard operators (`+`, `-`, `*`). These operations require matrices of the same dimensions.
* Algebraic matrix-matrix multiplication using the `mul()` function, which supports matrices of compatible dimensions (e.g., `float2x3 * float3x4`).
* Matrix-vector multiplication using `mul()`, where the vector can be interpreted as either a row or column vector depending on the parameter order:
  * `mul(v, m)` - v is interpreted as a row vector
  * `mul(m, v)` - v is interpreted as a column vector

For proper matrix multiplication, always use the `mul()` function. The `operator*` performs element-wise multiplication and should only be used when you want to multiply corresponding elements of same-sized matrices.

> Note: This differs from GLSL, where the `*` operator performs matrix multiplication. When porting code from GLSL or CUDA to Slang, you'll need to replace matrix multiplications using `*` with calls to `mul()`.

### Legacy Syntax

For compatibility with older codebases, the generic `matrix` type includes default values for `T`, `R`, and `C`, being declared as:

```hlsl
struct matrix<T = float, let R : int = 4, let C : int = 4> { ... }
```

This means that the bare name `matrix` may be used as a type equivalent to `float4x4`:

```hlsl
// All of these variables have the same type
matrix a;
float4x4 b;
matrix<float, 4, 4> c;
```
