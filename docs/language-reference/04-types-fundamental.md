Void Type
---------

The type `void` contains no data and has a single, unnamed, value.

A `void` value takes up no space, and thus does not affect the layout of types.
Formally, a `void` value behaves as if it has a size of zero bytes, and one-byte alignment.

Scalar Types
------------

### Boolean Type

The type `bool` is used to represent Boolean truth values: `true` and `false`.

The size of a `bool` varies across target platforms; programs that need to ensure a matching in-memory layout between targets should not use `bool` for in-memory data structures.
On all platforms, the `bool` type must be _naturally aligned_ (its alignment is its size).

### Integer Types

The following integer types are defined:

| Name          | Description |
|---------------|-------------|
| `int8_t`      | 8-bit signed integer |
| `int16_t`     | 16-bit signed integer |
| `int`         | 32-bit signed integer |
| `int64_t`     | 64-bit signed integer |
| `uint8_t`     | 8-bit unsigned integer |
| `uint16_t`    | 16-bit unsigned integer |
| `uint`        | 32-bit unsigned integer |
| `uint64_t`    | 64-bit unsigned integer |

All signed integers used two's complement representation.
All arithmetic operations on integers (both signed and unsigned) wrap on overflow/underflow.

All target platforms must support the `int` and `uint` types.
Specific [target platforms](../target-compatibility.md) may not support the other integer types.

All integer types are stored in memory with their natural size and alignment on all targets that support them.

### Floating-Point Types

The following floating-point type are defined:

| Name          | Description                   |
|---------------|-------------------------------|
| `half`        | 16-bit floating-point number (1 sign bit, 5 exponent bits, 10 fraction bits) |
| `float`       | 32-bit floating-point number (1 sign bit, 8 exponent bits, 23 fraction bits) |
| `double`      | 64-bit floating-point number (1 sign bit, 11 exponent bits, 52 fraction bits) |

All floating-point types are laid out in memory using the matching IEEE 754 standard format (`binary16`, `binary32`, `binary64`).
Target platforms may define their own rules for rounding, precision, denormals, infinities, and not-a-number values.

All target platforms must support the `float` type.
Specific [targets](../target-compatibility.md) may not support the other floating-point types.

All floating-point types are stored in memory with their natural size and alignment on all targets that support them.

