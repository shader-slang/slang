# Array Types

An *array type* is specifies an array of contiguously allocated elements. The array size may be either known
at compile-time or determined at runtime. The array size is always fixed during the lifetime of the array
object.

## Declaration Syntax

```hlsl
// (1) 1-dimensional array of length N
ElementType varName[N];
ElementType[N] varName;
var varName : ElementType[N];

// (2) N-element array of M-element arrays
//
// Note the order of length specifiers
ElementType varName[N][M];
ElementType[M][N] varName;
var varName : ElementType[M][N];

// (3) 1-dimensional array of unknown length
ElementType varName[];
ElementType[] varName;
var varName : ElementType[];

// (4) Unknown-length array of M-element arrays
ElementType varName[][M];
ElementType[M][] varName;
var varName : ElementType[M][];

// (5) Type alias for N-element array of M-element arrays
typealias ArrayType = ElementType[3][2];
```

where:
- `ElementType` is the type of the array element. The element type may not have an unknown length.
  - This implies that only the outermost dimension may have an unknown length.
- Array length expressions `N` and `M` are specialization-time constant integers.
  - When specified, array length must be at least 0.
- `varName` is the variable identifier

The declarations within each group are equivalent.

When using the `var` or `let` syntax for variable declaration, array length declarations may only appear in the
type.

An array with any dimension length of 0 is called a 0-length array. A 0-length array has 0
size. Instantiations of 0-length arrays are discarded. This includes variables, function parameters, and
struct data member. 0-length arrays may not be accessed during runtime using the subscript operator.

Restrictions for unknown-length arrays:
- When a non-const data member in a `struct` is an unknown-length array, it must be the last data member.
- An unknown-length array cannot be instantiated as a local variable unless the length can be inferred at
  compile-time in which case it becomes a known-length array.
- A function parameter with an unknown-length array cannot be `out` or `inout`.


> Remark 1: When using the C-style variable declaration syntax, array declarations binding to the variable
> identifier are applied from right to left. However, when binding to the type, the declarations are
> applied from left to right. Consider:
> ```hlsl
> int[2][3] arr[5][4];
> ```
> which is equivalent to:
> ```hlsl
> int[2][3][4][5] arr;
> ```

> Remark 2: Equivalent to `ElementType[N][M]` array type declaration would be
> `std::array<std::array<ElementType, N>, M>` in C++.

> Remark 3: Unlike in C and C++, array types in Slang do not decay to pointer types. The implication is that
> array objects are always passed as values in assignment and function calls, similar to `std::array`. To
> avoid memory copies when possible, the compiler attempts to optimize these as pass by constant references or
> pointers when the target supports it.

> Remark 4: 0-length arrays can be used to disable data members in `struct` types. See [Generics (TODO)](TODO)
> for further information.


### Element Count Inference for Unknown-Length Array

When a variable is declared with an unknown-length array type and it also includes an initial-value expression:
```hlsl
int a[] = { 0xA, 0xB, 0xC, 0xD };
```
the compiler will attempt to infer the element count based on the type and/or structure of the initial-value expression.
In the above case, the compiler will infer an element count of 4 from the structure of the initializer-list expression.
Thus, the preceding declaration is equivalent to:
```hlsl
int a[4] = { 0xA, 0xB, 0xC, 0xD };
```
A variable declared in this fashion semantically has a known-length array type and not an unknown-length array
type. The use of an unknown-length array type for the declaration is a convenience feature.


## Memory Layout

### Base Layout

The _stride_ of an array element type is the size of the element rounded up to the smallest multiple of its
alignment. The stride defines the byte offset difference between adjacent elements.

The base layout rules for an array type `T[]` or `T[N]`:

* Element `i` of the array starts at a byte offset relative to the array base address that is `i` times the
  element stride of the array.
* The alignment of the array type is the alignment of `T`.
* The size of an unknown-length array type is unknown.
* The size of a known-length array with zero elements is zero
* The size of a known-size array with a nonzero number `N` of elements is the size of `T` plus `N - 1` times the element stride of the array

### C-Style Layout

The C-style layout of an array type differs from the base layout in that the array size is `N` times the
element stride.

### D3D Constant Buffer Layout

The D3D constant buffer layout of an array type differs from the base layout in that the array size is `N`
times the element stride.
