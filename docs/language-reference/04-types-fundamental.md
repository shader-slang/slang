# Fundamental Types

The following types are collectively called the _fundamental types_:
- The `void` type
- The scalar Boolean type
- The scalar integer types
- The scalar floating point types


## Void Type

The type `void` contains no data and has a single unnamed value.

A function with return type `void` does not return a value.

Variables, arrays elements, or structure data members may not have type `void`.


## Scalar Types

### Boolean Type

Type `bool` is used to represent Boolean truth values: `true` and `false`.

The size of `bool` is target-defined. Similarly, the underlying bit patterns for `true` and `false` are
target-defined. The use of `bool` should be avoided when a specific in-memory layout of a data structure is
required. This includes data shared between different language targets even on the same device.


### Integer Types

The following integer types are defined:

| Name                 | Description             |
|----------------------|-------------------------|
| `int8_t`             | 8-bit signed integer    |
| `int16_t`            | 16-bit signed integer   |
| `int`, `int32_t`     | 32-bit signed integer   |
| `int64_t`            | 64-bit signed integer   |
| `uint8_t`            | 8-bit unsigned integer  |
| `uint16_t`           | 16-bit unsigned integer |
| `uint`, `uint32_t`   | 32-bit unsigned integer |
| `uint64_t`           | 64-bit unsigned integer |

All arithmetic operations on signed and unsigned integers wrap on overflow.

All target platforms support the `int`/`int32_t` and `uint`/`uint32_t` types. The support for other types depends on the target and target capabilities. See [target platforms](../target-compatibility.md) for details.

All integer types are stored in memory with their natural size and alignment on all target that support them.

### Floating-Point Types

The following floating-point type are defined:

| Name                  | Description                  | Precision (sign/exponent/significand bits) |
|-----------------------|------------------------------|--------------------------------------------|
| `half`, `float16_t`   | 16-bit floating-point number | 1/5/10                                     |
| `float`, `float32_t`  | 32-bit floating-point number | 1/8/23                                     |
| `double`, `float64_t` | 64-bit floating-point number | 1/11/52                                    |

Rules for rounding, denormals, infinite values, and not-a-number (NaN) values are generally
target-defined. IEEE 754 compliant targets adhere to the
[IEEE 754-2019](https://doi.org/10.1109/IEEESTD.2019.8766229) standard.

All targets support the `float`/`float32_t` type. Support for other types is target-defined. See
[target platforms](../target-compatibility.md) for details.


## Alignment and data layout

The size of a Boolean type is targed-defined. All other fundamental types have precisely defined sizes.

All fundamental types are _naturally aligned_. That is, their alignment is the same as their size.

All fundamental types use [little-endian](https://en.wikipedia.org/wiki/Endianness) representation.

All signed integers use [two's complement](https://en.wikipedia.org/wiki/Two%27s_complement) representation.

> Remark: Fundamental types in other languages are not always naturally aligned. In particular, the alignment
> of C type `uint64_t` on x86-32 is typically 4 bytes.
