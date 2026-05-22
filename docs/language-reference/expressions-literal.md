Literal Expressions
===================

All literal expressions are [r-values](expressions-value-categories.md).

## Boolean Literal Expressions

> *`BooleanLiteral`* = (**`'true'`** \| **`'false'`**)

Boolean literals represent [Boolean](types-fundamental.md#boolean) values `true` and `false`.


## Integer Literal Expressions

> *`IntegerLiteral`* = (<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`DecIntegerLiteralBody`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`HexIntegerLiteralBody`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`BinIntegerLiteralBody`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`OctIntegerLiteralBody`* )<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`IntegerSuffix`*?
>
> *`DecIntegerLiteralBody`* = <br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**`'0'`** \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`DecDigitNonZero`* *`DecDigit`*\*<br>
>
> *`DecDigit`* = **`<[0-9]>`**
>
> *`DecDigitNonZero`* = **`<[1-9]>`**
>
> *`HexIntegerLiteralBody`* = (**`'0x'`**\|**`'0X'`**) *`HexDigit`*+
>
> *`HexDigit`* = **`<[0-9A-Fa-f]>`**
>
> *`BinIntegerLiteralBody`* = (**`'0b'`**\|**`'0B'`**) *`BinDigit`*+
>
> *`BinDigit`* = **`'0'`** \| **`'1'`**
>
> *`OctIntegerLiteralBody`* = **`'0'`** *`OctDigit`*+
>
> *`OctDigit`* = **`<[0-7]>`**
>
> *`IntegerSuffix`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;( *`IntegerSuffixUnsigned`* *`IntegerSuffixWidth`*? ) \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;( *`IntegerSuffixWidth`* *`IntegerSuffixUnsigned`*? )
>
> *`IntegerSuffixUnsigned`* = **`'u'`** \| **`'U'`**
>
> *`IntegerSuffixWidth`* = **`'l'`** \| **`'L'`** \| **`'ll'`** \| **`'LL'`** \| **`'z'`** \| **`'Z'`**

An integer literal represents an [integer](types-fundamental.md#integer) value. It consists of two parts:

- the body, which consists of an optional prefix and digits.
- an optional suffix, which determines the type of the literal in conjunction with the value.

The body uses one of the following forms:

- decimal: a non-zero decimal digit followed by zero or more decimal digits, or plain `0`.
- hexadecimal: prefixed by `0x` or `0X` and followed by one or more hexadecimal digits.
- binary: prefixed by `0b` or `0B` and followed by zeroes and ones.
- octal: prefixed by `0` and followed by one or more octal digits. Octal integer literals are deprecated and
  supported only for compatibility; their use triggers a warning.

The integer literal suffix is optional. When specified, it consists of an unsigned specifier, a width
specifier, or both in either order. The unsigned specifier forces the literal to have an unsigned integer
type, and the width specifier selects the minimum width for the type.

The type of the literal is the first type from the following table that fits the value:

Suffix        | Decimal base                     | Hex, binary, octal bases
------------- | -------------------------------- | ---------------------------------------
(none)        | `int`, `int64_t`, `uint64_t`(\*) | `int`, `uint`, `int64_t`, `uint64_t`
`LL`          | `int64_t`, `uint64_t`(\*)        | `int64_t`, `uint64_t`
`U`/`UL`/`LU` | `uint`, `uint64_t`               | `uint`, `uint64_t`
`ULL`/`LLU`   | `uint64_t`                       | `uint64_t`
`Z`           | `intptr_t`                       | `intptr_t`
`UZ`/`ZU`     | `uintptr_t`                      | `uintptr_t`

Types marked with (\*) trigger a warning; they are intended only as a fallback to avoid undefined
behavior.

In addition, the following exceptions are made to allow expressing the smallest negative integer directly:

1. If the literal is `2147483648` or `2147483648L` and it is preceded by unary minus, the resulting value
   is `-2147483648` and the type is `int`.
2. If the literal is `9223372036854775808`, `9223372036854775808L`, or `9223372036854775808LL`, and it is
   preceded by unary minus, the resulting value is `-9223372036854775808` and the type is `int64_t`.
3. If the literal is `2147483648Z` (32-bit pointers) or `9223372036854775808Z` (64-bit pointers) and it is
   preceded by unary minus, the resulting value is `-2147483648` (32-bit pointers) or
   `-9223372036854775808` (64-bit pointers) and the type is `intptr_t`.

The following table summarizes the literal types given the suffix and the value.

Suffix                 | Base        | Value range                                 | Literal type
---------------------- | ----------- | ------------------------------------------- | -------------
(none)/`L`             | dec         | [0, 2147483647] (\*\*)                      | `int`
(none)/`L`             | dec         | [2147483648, 9223372036854775807] (\*\*)    | `int64_t`
(none)/`L`             | dec         | [9223372036854775808, 18446744073709551615] | `uint64_t` (\*)
(none)/`L`             | hex/bin/oct | [0x0, 0x7FFFFFFF]                           | `int`
(none)/`L`             | hex/bin/oct | [0x80000000, 0xFFFFFFFF]                    | `uint`
(none)/`L`             | hex/bin/oct | [0x100000000, 0x7FFFFFFFFFFFFFFF]           | `int64_t`
(none)/`L`             | hex/bin/oct | [0x8000000000000000, 0xFFFFFFFFFFFFFFFF]    | `uint64_t`
`LL`                   | dec         | [0, 9223372036854775807] (\*\*)             | `int64_t`
`LL`                   | dec         | [9223372036854775808, 18446744073709551615] | `uint64_t` (\*)
`LL`                   | hex/bin/oct | [0x0, 0x7FFFFFFFFFFFFFFF]                   | `int64_t`
`LL`                   | hex/bin/oct | [0x8000000000000000, 0xFFFFFFFFFFFFFFFF]    | `uint64_t`
`U`/`UL`/`LU`          | any         | [0, 4294967295]                             | `uint`
`U`/`UL`/`LU`          | any         | [4294967296, 18446744073709551615]          | `uint64_t`
`ULL`/`LLU`            | any         | [0, 18446744073709551615]                   | `uint64_t`
`Z` (32-bit)           | dec         | [0, 2147483647] (\*\*)                      | `intptr_t`
`Z` (32-bit)           | hex/bin/oct | [0x0, 0xFFFFFFFF]                           | `intptr_t`
`UZ`/`ZU` (32-bit)     | dec         | [0, 4294967295]                             | `uintptr_t`
`UZ`/`ZU` (32-bit)     | hex/bin/oct | [0x0, 0xFFFFFFFF]                           | `uintptr_t`
`Z` (64-bit)           | dec         | [0, 9223372036854775807] (\*\*)             | `intptr_t`
`Z` (64-bit)           | hex/bin/oct | [0x0, 0xFFFFFFFFFFFFFFFF]                   | `intptr_t`
`UZ`/`ZU` (64-bit)     | dec         | [0, 18446744073709551615]                   | `uintptr_t`
`UZ`/`ZU` (64-bit)     | hex/bin/oct | [0x0, 0xFFFFFFFFFFFFFFFF]                   | `uintptr_t`

(\*) marks a warning.

(\*\*) marks the value extension for the type when the literal is preceded by unary minus.

**Examples:**

```hlsl
0                       // decimal literal, type int
2147483647              // decimal literal, type int
2147483648              // decimal literal, type int64_t
18446744073709551615    // decimal literal, type uint64_t (warning)
18446744073709551616    // decimal literal, overflow error

-2147483648             // expression, type int
-2147483648LL           // expression, type int64_t

-9223372036854775808    // expression, type int64_t

0U                      // decimal literal, type uint
5000000000U             // decimal literal, type uint64_t
18446744073709551615U   // decimal literal, type uint64_t
18446744073709551616U   // decimal literal, overflow error

1000                    // decimal literal, type int
1000L                   // decimal literal, type int
1000LL                  // decimal literal, type int64_t

1000U                   // decimal literal, type uint
1000UL                  // decimal literal, type uint
1000ULL                 // decimal literal, type uint64_t

0x12345678              // hexadecimal literal, type int
0xDEADBEEF              // hexadecimal literal, type uint
0x1234567890ABCDEF      // hexadecimal literal, type int64_t
0xFEDCBA0987654321      // hexadecimal literal, type uint64_t

0b11010111              // binary literal, type int
0b11010111U             // binary literal, type uint

0377                    // octal literal, type int (warning)

0x10494810000UZ         // hexadecimal literal, type uintptr_t
```

> 📝 **Remark 1:** Hexadecimal, binary, and octal literals whose deduced type is unsigned (`uint` or
> `uint64_t`) and that have no `U` or `Z` suffix may be implicitly converted to the corresponding signed
> integer type without triggering diagnostics. This allows expressions such as `int x = 0xFFFFFFFF;`. The
> binary representation is not changed by the conversion. See
> [expression type conversions](expressions-conversions.md) for details.

> 📝 **Remark 2:** Integer literal types follow the C++11 rules, with additional special-case handling for
> minimum integer values preceded by unary minus.


## Floating-Point Literal Expressions

> *`FloatLiteral`* = (<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`DecFloatLiteralBody`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`HexFloatLiteralBody`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`FloatLiteralInfinity`* )<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`FloatSuffix`*?
>
> *`DecFloatLiteralBody`* = <br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DecFloatLiteralBodyForm1`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DecFloatLiteralBodyForm2`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DecFloatLiteralBodyForm3`*
>
> *`DecFloatLiteralBodyForm1`* = *`DecDigit`*+ *`DecExponent`*
>
> *`DecFloatLiteralBodyForm2`* = *`DecDigit`*+ **`'.'`** *`DecExponent`*?
>
> *`DecFloatLiteralBodyForm3`* = *`DecDigit`*\* **`'.'`** *`DecDigit`*+ *`DecExponent`*?
>
> *`DecExponent`* = (**`'e'`** \| **`'E'`**) (**`'+'`** \| **`'-'`**)? *`DecDigit`*+
>
>
> *`HexFloatLiteralBody`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'0x'`**\|**`'0X'`**) (<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`HexFloatLiteralBodyForm1`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`HexFloatLiteralBodyForm2`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`HexFloatLiteralBodyForm3`* )
>
> *`HexFloatLiteralBodyForm1`* = *`HexDigit`*+ *`HexExponent`*
>
> *`HexFloatLiteralBodyForm2`* = *`HexDigit`*+ **`'.'`** *`HexExponent`*
>
> *`HexFloatLiteralBodyForm3`* = *`HexDigit`*\* **`'.'`** *`HexDigit`*+ *`HexExponent`*
>
> *`HexExponent`* = (**`'p'`** \| **`'P'`**) (**`'+'`** \| **`'-'`**)? *`DecDigit`*+
>
> *`FloatLiteralInfinity`* = **`'#INF'`**
>
> *`FloatSuffix`* = *`FloatSuffixF16`* \| *`FloatSuffixF32`* \| *`FloatSuffixF64`*
>
> *`FloatSuffixF16`* = **`'h'`** \| **`'H'`** \| **`'hf'`** \| **`'HF'`** \| **`'fh'`** \| **`'FH'`**
>
> *`FloatSuffixF32`* = **`'f'`** \| **`'F'`**
>
> *`FloatSuffixF64`* = **`'l'`** \| **`'L'`** \| **`'lf'`** \| **`'LF'`** \| **`'fl'`** \| **`'FL'`**

A floating-point literal represents a [floating-point](types-fundamental.md#floating) value. The numeric form
consist of three parts:

- the body, which can be either decimal or hexadecimal.
- an exponent, which is optional in decimal form when the body contains a decimal separator.
- an optional suffix, which determines the type of the literal. If omitted, the type is `float`.

The decimal body has two variants:

1. Decimal digits and exponent. (*`DecFloatLiteralBodyForm1`*)
2. Decimal digits separated by a decimal separator, and an optional exponent. (*`DecFloatLiteralBodyForm2`*
   and *`DecFloatLiteralBodyForm3`*)

The decimal digits and the optional separator form the decimal significand. The value is significand \* *10^x*
where *x* is the number with the optional sign in *`DecExponent`* or 0 if *`DecExponent`* is not specified.

The hexadecimal body form has one or more hexadecimal digits, optional radix separator, and a mandatory
hexadecimal exponent. (*`HexFloatLiteralBodyForm1`*, *`HexFloatLiteralBodyForm2`*, *`HexFloatLiteralBodyForm3`*)

The hexadecimal digits and the optional separator form the hexadecimal significand. The value is *significand* \* *2^y*
where *y* is the number with the optional sign in *`HexExponent`*. Note that the number 

The non-numeric form `#INF` represents the positive infinity. (*`FloatLiteralInfinity`)


**Examples:**

```hlsl
TODO
```

> 📝 **Remark 1:** A floating-point literal expression without a suffix has type `float`.

## String Literal Expressions (todo)

A string literal expressions consists of one or more string literal tokens in a row:

```hlsl
"This" "is one" "string"
```
