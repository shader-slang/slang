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

```hlsl
MyStruct s = (MyStruct) 0;
```

The semantics of such a cast are equivalent to initialization from an empty initializer list:

```hlsl
MyStruct s = {};
```

### Cast to Void

A cast to the [void](types-fundamental.md) is a special kind of cast. The cast itself is a no operation
producing a `void` value. The primary use of a `void` cast is mark a value consumed, suppressing related
warnings. See also [attribute \[NoDiscard\]](../../../core-module-reference/attributes/nodiscard-02.html).



## Implicit Type Conversion



## Bit Cast

