# Expression Type Conversions

A type conversion changes the type of a value. For example, an `int` may be converted to a `float`, and vice
versa.

The following type conversion

- _Explicit type conversion_ occurs with the _cast expression_ or a _constructor expression_ (see
  below). Explicit type conversion is used to convert a value to a specific type.
- _Implicit type conversion_ occurs when the value of certain type is passed to a context that requires another
  type. Implicit type conversion is sometimes referred as _type coercion_.
- A _bit cast_ reinterprets the underlying bit pattern of a value of one type as a value of another type.


## Cast Expression

**Grammar:**

> **`'('`** *`type-expr`* **`')'`**) *`base-expr`*
>
> > *`type-expr`* **`'('`** *`base-expr`* **`')'`**)

A _cast expression_ converts a value (*`base-expr`*) to the desired type (*`type-expr`*). For example, `(uint32_t)5`.

An _initializer expression_ creates a new value of the desired type (*`type-expr`*) from (*`base-expr`*). For example,
`float(5)`.

For fundamental types, cast expressions and single-argument initializer expressions have the same
semantics. For user-defined types, a cast expression invokes the single-argument initializer of the target
type.

The conversion rules between fundamental types are as follows:

| Source type (S)     | Target type (T)     | Description                                                                  |
|---------------------|---------------------|------------------------------------------------------------------------------|
| bool                | integer type        | Same as `(src ? T(1) : T(0))`                                                |
| bool                | floating-point type | Same as `(src ? T(1.0) : T(0.0))`                                            |
| integer type        | bool                | Same as `(src != S(0))`                                                      |
| integer type        | integer type        | See below.                                                                   |
| integer type        | floating-point type | Rounded to a representable value (round towards zero).                       |
| floating-point type | bool                | Same as `(src != S(0.0))`                                                    |
| floating-point type | integer type        | Rounded to a representable value (round towards zero).                       |
| floating-point type | floating-point type | Rounded to a representable value. Rounding is implementation-defined.        |


**The procedure for integer-integer conversions:**

1. Match the width of value with the target type.
   - If the source value type is narrower than the target type, zero-extend (unsigned source type) or
     sign-extend (signed source type) the source value such that the widths match.
   - If the source value type width equals to the target type width, do nothing.
   - If the source value type width is less than the target type width, discard the high bits of the source
     value such that the widths match.
2. Change the signedness of the value if necessary to match the signedness of the target type.
   - This step does not change the bit representation of the value.

Note that when the source value can be represented with the target type, the value does not change in
conversion.

If the integer-to-float or float-to-integer conversion is not representable by the target type after rounding,
the behavior is [undefined](basics-behavior.md).

As a compatibility feature for legacy code, Slang supports using a cast where the base expression is an
integer literal zero and the target type is a user-defined [structure](types-struct.md) type:

> 📝 **Remark:** As a compatibility feature for legacy code, Slang supports a cast from literal 0 to a
> user-defined [structure](types-struct.md) type. This is equivalent to initializing the structure with an
> empty initializer list.
>
> ```hlsl
> MyStruct s = (MyStruct) 0;
>
> // is same as
>
> ```hlsl
> MyStruct s = {};
> ```
>
> This is a deprecated feature and subject to be removed in a future Slang language version.
> See also [GitHub issue 12045](https://github.com/shader-slang/slang/issues/12045).

### Cast to Void

A cast to the [void](types-fundamental.md) is a no operation cast producing a `void` value. The primary use of
a `void` cast is mark the value consumed, suppressing related warnings. See also
[attribute \[NoDiscard\]](../../../core-module-reference/attributes/nodiscard-02.html).

### Casts Between Scalar, Vector, and Matrix Types


## Implicit Type Conversion

Implicit type conversion occurs when the type of a value does not match with the required type, there is an
explicit conversion available, and implicit conversion is allowed.

The following implicit type conversions are allowed:

- `bool` to an integer type
- integer type to a wider integer type, same signedness (aka. integer promotion)
- `half` to `float`
- scalar `T` to `vector<T, N>` (where `N` is any legal value)
- scalar `T` to `matrix<T, R, C>` (where `R` and `C` are any legal values)
- `vector<T, N>` to `vector<U, N>` where conversion `T` &rarr; `U` is allowed
- `matrix<T, R, C>` to `matrix<U, R, C>` where conversion `T` &rarr; `U` is allowed
- a type to its conformance type
- `none` or value of `T` to `Optional<T>`
- `nullptr` to any pointer type
- sized array to unsized array of same element type
- `enum` type to its tag type

The following implicit type conversions are allowed but not recommended. These implicit type conversions
trigger a warning.

- `bool` to a floating point type
- integer type to a `bool`
- integer type to a narrower integer type, except integer literals whose values fit in the narrower type
- integer type to same width integer type with different signedness, except integer literals whose values fit
  in the target type
- integer type to a floating-point type and vice versa
- floating-point type to a narrower float type
- float-point type to a double type (possible performance issue)
- vector to vector and matrix to matrix where the target element type is narrower or has different signedness
- integer vector to a floating-point vector and vice versa
- `enum` to integer type other than its tag type


> 📝 **Remark:** Some usual contexts for implicit type conversions:
>
> - Assigning a value of one type to a variable of another type
> - [Function](expressions-operators.md) call where the argument type does not match the parameter type
> - [Operator](expressions-operators.md) call where the argument type does not match the parameter type
> - [Generic argument application](generics.md) where the argument type does not match the generic parameter type



## Bit Cast

