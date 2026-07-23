# Expression Type Conversions

A type conversion changes the type of a value. For example, an `int` may be converted to a `float`, and vice
versa.

Slang supports the following kinds of type conversions:

- _Explicit type conversion_ occurs with a _cast expression_ or an _initializer expression_ (see
  below). Explicit type conversion is used to convert a value to a specific type.
- _Implicit type conversion_ occurs when a value of a certain type is passed to a context that requires another
  type. Implicit type conversion is sometimes referred to as _type coercion_.
- A _bit cast_ reinterprets the underlying bit pattern of a value of one type as a value of another type.


## Cast Expression

**Grammar:**

> **`'('`** *`type-expr`* **`')'`** *`base-expr`*
>
> *`type-expr`* **`'('`** *`base-expr`* **`')'`**

A _cast expression_ converts a value (*`base-expr`*) to the desired type (*`type-expr`*). For example, `(uint32_t)5`.

An _initializer expression_ creates a new value of the desired type (*`type-expr`*) from (*`base-expr`*). For example,
`float(5)`.

For fundamental types, cast expressions and single-argument initializer expressions have the same
semantics. For user-defined types, a cast expression invokes the single-argument initializer of the target
type.

The conversion rules between fundamental types are as follows:

| Source type (S)     | Target type (T)     | Description                                                           |
|---------------------|---------------------|-----------------------------------------------------------------------|
| bool                | integer type        | Same as `(src ? T(1) : T(0))`                                         |
| bool                | floating-point type | Same as `(src ? T(1.0) : T(0.0))`                                     |
| integer type        | bool                | Same as `(src != S(0))`                                               |
| integer type        | integer type        | See below.                                                            |
| integer type        | floating-point type | Rounded to a representable value. Rounding is implementation-defined. |
| floating-point type | bool                | Same as `(src != S(0.0))`                                             |
| floating-point type | integer type        | Rounded to a representable value (round towards zero).                |
| floating-point type | floating-point type | Rounded to a representable value. Rounding is implementation-defined. |


**The procedure for integer-integer conversions:**

1. Match the width of the value with the target type.
   - If the source value type is narrower than the target type, zero-extend (unsigned source type) or
     sign-extend (signed source type) the source value such that the widths match.
   - If the source value type width equals the target type width, do nothing.
   - If the source value type is wider than the target type, discard the high bits of the source
     value such that the widths match.
2. Change the signedness of the value if necessary to match the signedness of the target type.
   - This step does not change the bit representation of the value.

Note that when the source value can be represented with the target type, the value does not change in
conversion.

If the integer-to-float or float-to-integer conversion is not representable by the target type after rounding,
the behavior is [undefined](basics-behavior.md).

> 📝 **Remark:** As a compatibility feature for legacy code, Slang &le;2026 has special semantics for a cast
> from literal 0 to a user-defined [structure](types-struct.md) type. This is equivalent to initializing the
> structure with a default initializer.
>
> The special semantics are removed in Slang 2027. In Slang 2027, a cast from literal 0 is a regular
> conversion, and it invokes the single-argument initializer of the target type.
>
> ```hlsl
> MyStruct s = (MyStruct)0;
>
> // In Slang 2026 and previous versions, the above is the same as
> MyStruct s = MyStruct();
>
> // From Slang 2027 onward, the cast from literal 0 is equivalent to
> MyStruct s = MyStruct(0);
> ```
>
> See also GitHub issue [#12045](https://github.com/shader-slang/slang/issues/12045).

#### Examples

```hlsl
StructuredBuffer<double> doubleInputs;
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int16_t vali16 = 0x8000;
    uint16_t valu16 = 0x8000U;
    output[0] = (uint)vali16; // writes 0xFFFF8000 (sign extension)
    output[1] = (uint)valu16; // writes 0x00008000 (zero extension)

    uint64_t valu64 = 0x123456789ABCDEFLLU;
    output[2] = (uint)valu64; // writes 0x89ABCDEF (discard high bits)

    if ((bool)vali16)
    {
        // branch executed if vali16 != 0 (which it is)
        output[3] = 123U;
    }

    // double -> uint conversion (truncates decimal fraction)
    output[4] = (uint)doubleInputs[0];

    // double -> uint conversion using the initializer syntax
    output[5] = uint(doubleInputs[1]);
}
```

### Cast to Void

A cast to [`void`](types-fundamental.md) is a no-op cast producing a `void` value. The primary use of
a `void` cast is to mark the value as consumed, suppressing related warnings. See also
[attribute \[NoDiscard\]](../../../core-module-reference/attributes/nodiscard-02.html).

#### Examples

```hlsl
RWStructuredBuffer<uint> output;

enum StatusCode
{
    Success = 0,
    Overflow = 1
}

[NoDiscard] StatusCode incrementValue(inout uint val)
{
    uint prevVal = val;

    ++val;

    // overflow detection
    return prevVal < val ? StatusCode.Success : StatusCode.Overflow;
}

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // ignore overflow status code
    (void)incrementValue(output[0]);
}
```

### Conversions Between Scalar, Vector, and Matrix Types

A scalar can be cast to a vector or matrix type. In this conversion, the scalar value is used to
populate every element of the vector or matrix.

A vector can be constructed from elements and smaller vectors. The elements are concatenated to form the new
vector. If the target vector type has more elements than supplied (but at least 2 were supplied), the newly
constructed vector will be tail-padded by 0. However, see the warning below.

A matrix can be constructed from vectors in the following ways:

- `R` row vectors `vector<T,C>` to a matrix `matrix<V,R,C>` using the constructor syntax. `V` must be
  convertible from `T`.
- Matrix extension with a row vector using the constructor syntax: `matrix<T,R-1,C>` and `vector<U,C>` &rarr;
  `matrix<V,R,C>`. `V` must be convertible from `T` and `U`.
- Conversions between `matrix<T,2,2>` and `vector<T,4>` types with the cast or constructor syntax. The vector
  elements are mapped to matrix elements as follows:
  - vector elements 0, 1 &harr; matrix row 0
  - vector elements 2, 3 &harr; matrix row 1

For details, see [vector initialization functions](../../../core-module-reference/types/vector/init.html) and
[matrix initialization functions](../../../core-module-reference/types/matrix/init.html).


> ⚠️ **Warning:** The construction of a 4-dimensional vector from a 2-dimensional vector and an element is
> currently inconsistent with the tail-padding by 0 semantics. This is tracked by GitHub issue
> [12093](https://github.com/shader-slang/slang/issues/12093).

> ⚠️ **Warning:** Constructing a vector using an initializer list with a single element is equivalent to
> initializing with a scalar. That is:
>
> ```hlsl
> int4 v0 = 1;        // { 1, 1, 1, 1 } - duplicate
> int4 v1 = { 1 };    // { 1, 1, 1, 1 } - duplicate
> int4 v2 = { 1, 1 }; // { 1, 1, 0, 0 } - tail-pad with 0
> ```

#### Examples

```hlsl
RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vector<float,2> v0;

    // explicit conversion from a scalar 2.0 to vector
    v0 = (vector<float,2>)2.0;

    // then assign 1.0 to the first element of v0
    v0.x = 1.0;

    vector<float,2> v1 = { 3, 4 };
    vector<float,2> v2 = { 5, 6 };
    vector<float,2> v3 = { 7, 8 };

    // construct a matrix from 3 row vectors
    matrix<float,3,2> m_3x2 = { v0, v1, v2 };

    // construct a matrix from a smaller matrix
    // and an additional row vector
    matrix<float,4,2> m_4x2 = { m_3x2, v3 };

    // write out m_4x2
    output[0] = m_4x2[0][0]; // 1.0
    output[1] = m_4x2[0][1]; // 2.0
    output[2] = m_4x2[1][0]; // 3.0
    output[3] = m_4x2[1][1]; // 4.0
    output[4] = m_4x2[2][0]; // 5.0
    output[5] = m_4x2[2][1]; // 6.0
    output[6] = m_4x2[3][0]; // 7.0
    output[7] = m_4x2[3][1]; // 8.0

    matrix<float,2,2> m_2x2 = { 9, 10, 11, 12 };
    vector<float,4> v4 = (vector<float,4>)m_2x2;
    output[8] = v4.x;  // 9.0
    output[9] = v4.y;  // 10.0
    output[10] = v4.z; // 11.0
    output[11] = v4.w; // 12.0

    // construct a 4-dim vector from a 2-dim vector
    // and two elements
    vector<float,2> v5 = { 14, 15 };
    vector<float,4> v6 = { 13, v5, 16 };
    output[12] = v6.x; // 13.0
    output[13] = v6.y; // 14.0
    output[14] = v6.z; // 15.0
    output[15] = v6.w; // 16.0
}
```

## Implicit Type Conversion

Implicit type conversion occurs when the type of a value does not match the required type, there is an
explicit conversion available, and implicit conversion is allowed.

The following implicit type conversions are allowed:

- `bool` to an integer type
- integer type to a wider integer type, same signedness (aka. integer promotion)
- `half` to `float`
- scalar `T` to `vector<T, N>` (where `N` is any legal value)
- scalar `T` to `matrix<T, R, C>` (where `R` and `C` are any legal values)
- `vector<T, N>` to `vector<U, N>` where conversion `T` &rarr; `U` is allowed
- `matrix<T, R, C>` to `matrix<U, R, C>` where conversion `T` &rarr; `U` is allowed
- a type to an interface it conforms to
- `none` or a value of `T` to `Optional<T>`
- `nullptr` to any pointer type
- sized array to unsized array of same element type
- `enum` type to its tag type
- an initializer list to a type with an initializer that accepts the arguments in the list

The following implicit type conversions are allowed but not recommended. These implicit type conversions
trigger a diagnostic warning.

- `bool` to a floating-point type
- integer type to a `bool`
- integer type to a narrower integer type, except integer literals whose values fit in the narrower type
- integer type to same width integer type with different signedness, except integer literals whose values fit
  in the target type
- integer type to a floating-point type and vice versa
- floating-point type to a narrower float type
- floating-point type to a double type (possible performance issue)
- vector to vector and matrix to matrix where the target element type is narrower or has different signedness
- integer vector to a floating-point vector and vice versa
- `enum` to integer type other than its tag type

> 📝 **Remark:** Some common contexts for implicit type conversions:
>
> - Assigning a value of one type to a variable of another type
> - [Function](expressions-operators.md) call where the argument type does not match the parameter type
> - [Operator](expressions-operators.md) call where the argument type does not match the parameter type
> - [Generic argument application](generics.md) where the argument type does not match the generic parameter type

> ⚠️ **Warning:** In Slang 2025 and earlier, `vector<T, 4>` allowed initialization from
> `(vector<T, 2>, T)` and `(T, vector<T, 2>)` arguments due to the `T` &rarr; `vector<T, 2>` implicit
> conversion. This has been removed in Slang 2026.
> See GitHub issue [#12093](https://github.com/shader-slang/slang/issues/12093) for details.

#### Examples

**Scalar to vector, scalar to matrix**
```slang
RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vector<float,4> v0;

    // implicit conversion from scalar to vector
    v0 = 1.0; // { 1.0, 1.0, 1.0, 1.0 }

    matrix<float, 3, 2> m0;

    // implicit conversion from scalar to matrix
    m0 = 2.0; // { 2.0, 2.0,  2.0, 2.0,  2.0, 2.0 }

    output[0] = v0.x; // 1.0
    output[1] = v0.x; // 1.0
    output[2] = v0.x; // 1.0
    output[3] = v0.x; // 1.0
    output[4] = m0[0][0]; // 2.0
    output[5] = m0[0][1]; // 2.0
    output[6] = m0[1][0]; // 2.0
    output[7] = m0[1][1]; // 2.0
    output[8] = m0[2][0]; // 2.0
    output[9] = m0[2][1]; // 2.0
}
```

**Value and `none` to Optional:**
```slang
StructuredBuffer<int64_t> input;
RWStructuredBuffer<int> output;

static int index = 0;

void maybeWriteToOutput(Optional<int> optVal)
{
    // the special 'if' syntax for checking whether
    // optVal has a value and assigning the value
    if (let val = optVal)
    {
        // ok, we have a value. Let's write it
        output[index++] = val;
    }
}

// Return an optional with a value if val64 is in bounds
// for 'int', otherwise return an optional without a value
Optional<int> boundsCheckForInt32(int64_t val64)
{
    int val32 = (int)val64;

    // check whether the value survived the cast
    if (val32 == val64)
    {
        // survived, cast the value implicitly as
        // Optional<int> holding the value
        return val32;
    }
    else
    {
        // didn't survive, cast 'none' implicitly as
        // Optional<int> holding nothing
        return none;
    }
}

[numthreads(1,1,1)]
void main()
{
    // write 256 entries from input to output, skipping
    // input values that are out of bounds
    for (uint i = 0; i < 256; ++i)
        maybeWriteToOutput(boundsCheckForInt32(input[i]));
}
```

**Enum to tag type**
```slang
RWStructuredBuffer<uint> output;

enum TestEnum : uint
{
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
}

[numthreads(1,1,1)]
void main()
{
    // explicit cast is not needed here, since the enum
    // tag type matches the type of the assigned variable
    output[0] = TestEnum.Zero;  // 0
    output[1] = TestEnum.One;   // 1
    output[2] = TestEnum.Two;   // 2
    output[3] = TestEnum.Three; // 3

    // however, a cast would be needed here, since this
    // is not an exact match
    // uint64_t v = TestEnum.Zero;
}
```

**Implicit initializer list conversions:**
```slang
RWStructuredBuffer<int> output;

int2 someFunc(int3 v)
{
    // int2 has an (int, int) initializer, so we can create
    // the return value using an { int, int } initializer list.
    //
    // The list is converted to int2 by invoking int2.__init(int, int).
    return { v.x + v.y + v.z, 1001 };
}

void main()
{
    int2 a = { 1, 2 };
    int b = 3;

    // Assign new values to 'a', overriding the previous value.
    // Initializer list { int, int } is converted to int2 when
    // invoking the assignment operator function.
    a = { 9, 5 };

    // Since int3 has an (int2, int) initializer, { a, b } is converted
    // to int3 when calling someFunc
    a = someFunc({ a, b });

    output[0] = a.x; // 9 + 5 + 3 = 16
    output[1] = a.y; // 1001
}
```

## Bit Cast and Reinterpret Cast

A bit cast reinterprets an existing bit pattern as another type of the same size. A bit cast is invoked using
the [bit\_cast](../../../core-module-reference/global-decls/bit_cast.html) function. It is generally
[implementation-defined behavior](basics-behavior.md) how values are encoded as underlying bit
patterns. However, an application can reasonably expect the following:

- Signed integers are two's complement.
- Floating-point types `half`, `float`, and `double` use the IEEE 754 encoding. Other floating-point types use
  their respective encodings.

The standard library also offers the following concrete HLSL-compatibility conversion and bit-cast functions:

- [asdouble](../../../core-module-reference/global-decls/asdouble.html)
- [asfloat](../../../core-module-reference/global-decls/asfloat.html)
- [asfloat16](../../../core-module-reference/global-decls/asfloat16.html)
- [asint](../../../core-module-reference/global-decls/asint.html)
- [asuint](../../../core-module-reference/global-decls/asuint.html)
- [asuint16](../../../core-module-reference/global-decls/asuint16.html)
- [f16tof32](../../../core-module-reference/global-decls/f16tof32.html)
- [f32tof16](../../../core-module-reference/global-decls/f32tof16.html)

A reinterpret cast is more general, and it allows reinterpreting a bit pattern of a different size than the
target type. A reinterpret cast is invoked using the
[reinterpret](../../../core-module-reference/global-decls/reinterpret.html) function, and it uses the same
union type emulation as [interface-conforming variants](types-interface.md). That is, the source value is
packed into an AnyValue struct, which is then unpacked as the target type.


#### Examples

```slang
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float f = 1.0#INF; // positive infinity

    // writes the bit pattern for 32-bit floating-point positive infinity
    output[0] = bit_cast<uint>(f); // 0x7F800000U
}
```
