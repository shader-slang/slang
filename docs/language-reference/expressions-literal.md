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
  supported only for backwards compatibility; their use triggers a warning. Note that the literal `0` alone
  is decimal, while `00`, `01`, etc. are (deprecated) octal literals.

The integer literal suffix is optional. When specified, it consists of an unsigned specifier, a width
specifier, or both in either order. The unsigned specifier forces the literal to have an unsigned integer
type, and the width specifier selects the minimum width for the type.

The type of the literal is the first type from the following table that fits the value:

Suffix        | Decimal base                     | Hex, binary, octal bases
------------- | -------------------------------- | ---------------------------------------
(none)/`L`    | `int`, `int64_t`, `uint64_t`(\*) | `int`, `uint`, `int64_t`, `uint64_t`
`LL`          | `int64_t`, `uint64_t`(\*)        | `int64_t`, `uint64_t`
`U`/`UL`/`LU` | `uint`, `uint64_t`               | `uint`, `uint64_t`
`ULL`/`LLU`   | `uint64_t`                       | `uint64_t`
`Z`           | `intptr_t`                       | `intptr_t`
`UZ`/`ZU`     | `uintptr_t`                      | `uintptr_t`

Types marked with (\*) trigger a warning; they are intended only as a fallback to silently accept values
that would otherwise overflow the signed integer type.

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

(\*\*) marks rows whose value range is extended by one when the literal is preceded by unary minus, per
the special cases listed above (for example, the `int` row also accepts `2147483648` when negated, since
`-2147483648` is representable as `int`).

**Examples:**

```hlsl
0                       // decimal literal, type int
2147483647              // decimal literal, type int
2147483648              // decimal literal, type int64_t
18446744073709551615    // decimal literal, type uint64_t (warning)
18446744073709551616    // decimal literal, overflow error

-2147483648             // expression, type int
-2147483648LL           // expression, type int64_t

-9223372036854775808    // expression, type int64_t (no warning per special case)

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

0x10494810000UZ         // hexadecimal literal, type uintptr_t (64-bit pointers only;
                        //                                     overflows on 32-bit)
```

> 📝 **Remark 1:** Hexadecimal, binary, and octal literals whose deduced type is unsigned (`uint` or
> `uint64_t`) and that have no `U` or `Z` suffix may be implicitly converted to the corresponding signed
> integer type without triggering a narrowing-conversion warning. This allows expressions such as
> `int x = 0xFFFFFFFF;`. The binary representation is not changed by the conversion. See
> [expression type conversions](expressions-conversions.md) for details.

> 📝 **Remark 2:** Integer literal types follow the C++11 rules, with additional special-case handling for
> minimum integer values preceded by unary minus.

## Floating-Point Literal Expressions

> *`FloatLiteral`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;( *`DecFloatLiteralBody`* \| *`HexFloatLiteralBody`* )<br>
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
> *`DecExponent`* = *`DecExponentNumeric`* \| *`InfinityExponent`*
>
> *`DecExponentNumeric`* = (**`'e'`** \| **`'E'`**) (**`'+'`** \| **`'-'`**)? *`DecDigit`*+
>
> *`InfinityExponent`* = **`'#INF'`**
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
> *`HexExponent`* = *`HexExponentNumeric`* \| *`InfinityExponent`*
>
> *`HexExponentNumeric`* = (**`'p'`** \| **`'P'`**) (**`'+'`** \| **`'-'`**)? *`DecDigit`*+
>
> *`FloatSuffix`* = *`FloatSuffixF16`* \| *`FloatSuffixF32`* \| *`FloatSuffixF64`*
>
> *`FloatSuffixF16`* = **`'h'`** \| **`'H'`** \| **`'hf'`** \| **`'HF'`** \| **`'fh'`** \| **`'FH'`**
>
> *`FloatSuffixF32`* = **`'f'`** \| **`'F'`**
>
> *`FloatSuffixF64`* = **`'l'`** \| **`'L'`** \| **`'lf'`** \| **`'LF'`** \| **`'fl'`** \| **`'FL'`**

A floating-point literal represents a [floating-point](types-fundamental.md#floating) value. The numeric form
consists of three parts:

- the body, which can be either decimal or hexadecimal.
- an exponent, which is optional in decimal form when the body contains a decimal separator.
- an optional suffix, which determines the type of the literal. If omitted, the type is `float`.

The decimal body has two variants:

1. Decimal digits and exponent. (*`DecFloatLiteralBodyForm1`*)
2. Decimal digits separated by a decimal separator, and an optional exponent. (*`DecFloatLiteralBodyForm2`*
   and *`DecFloatLiteralBodyForm3`*)

The decimal digits and the optional separator form the decimal significand. The value is
*significand \* 10^x*, where *x* is the signed decimal number given by *`DecExponentNumeric`*, or 0
if no exponent is specified.

The hexadecimal body consists of hexadecimal digits, an optional radix separator, and a mandatory
hexadecimal exponent (*`HexFloatLiteralBodyForm1`*, *`HexFloatLiteralBodyForm2`*,
*`HexFloatLiteralBodyForm3`*).

The hexadecimal digits and the optional separator form the hexadecimal significand. The value of the
literal is *significand \* 2^y*, where *y* is the signed decimal number given by
*`HexExponentNumeric`*. Note that the exponent is always written in decimal.

In either decimal or hexadecimal form, using `#INF` as the exponent signifies that the literal value is
positive infinity. The digits before the exponent are ignored. Negative infinity is expressed by preceding
the literal with unary minus, e.g. `-1#INFf`.


**Examples:**

```hlsl
123.0         // 32-bit float, value 123.0
123.          // 32-bit float, value 123.0
.5            // 32-bit float, value 0.5
123e3         // 32-bit float, value 123000.0
123e+3        // 32-bit float, value 123000.0
123e-3        // 32-bit float, value 0.123   (not exact)
1.23e2        // 32-bit float, value 123.0

0x123p4       // 32-bit float, value 4656.0  (= 291 * 2^4)
0xC8p-4       // 32-bit float, value 12.5    (= 200 * 2^-4)
0xC.8p0       // 32-bit float, value 12.5    (= 12 + 8 * 2^-4)

123.0lf       // 64-bit float, value 123.0
123.0hf       // 16-bit float, value 123.0

1#INFhf       // 16-bit float, positive infinity
1#INFf        // 32-bit float, positive infinity
1#INFlf       // 64-bit float, positive infinity

123f          // error: '123' is an integer literal and 'f' is not a valid
              //        integer suffix. Write '123.f' or '123e0f' for a float.
```

> 📝 **Remark 1:** A floating-point literal expression without a suffix has type `float`.

> 📝 **Remark 2:** The current implementation does not fully conform to the language manual.
> This is tracked by GitHub issue [#11276](https://github.com/shader-slang/slang/issues/11276).


## String Literal Expressions

> *`StringLiteral`* = *`StringLiteralToken`*+
>
> *`StringLiteralToken`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DQuotedString`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`RawString`*
>
> *`DQuotedString`* = **`'"'`** *`DStringChar`*\* **`'"'`**
>
> *`DStringChar`* = <br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharUnquoted`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuoted`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuotedOctal`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuotedHex`*
>
> *`DStringCharUnquoted`* = **`<[^\\"[:newline:]]>`**
>
> Note: [:newline:] consists of characters `\r` and `\n`. (See the escape sequence table below.)
>
> *`DStringCharQuoted`* = **`<\\[\\'"?abfnrtv]>`**
>
> *`DStringCharQuotedOctal`* = **`<\\[0-7]{1,3}>`**
>
> *`DStringCharQuotedHex`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`<\\x[0-9A-Fa-f]+>`** \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`<\\x\{[0-9A-Fa-f]+\}>`**
>
> *`RawString`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'R"'`** *`RawStringDelim`* **`'('`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`RawStringContent`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`')'`** *`RawStringDelim`* **`'"'`**
>
> *`RawStringDelim`* = *`RawStringDelimChar`*\*
>
> *`RawStringDelimChar`* = **`<[^"()[:space:]]>`**
>
> Note: [:space:] consists of characters ` `, `\t`, `\r`, `\n`, `\v`, and `\f`. (See the escape
> sequence table below.)
>
> *`RawStringContent`* is the longest sequence of characters that does not contain
> **`')'`** *`RawStringDelim`* **`'"'`**, where *`RawStringDelim`* is the same delimiter
> token that opened the raw string.

A string literal represents a sequence of 8-bit characters. Its type is
[String](../../../core-module-reference/types/string-0/index.html). The underlying data format is unspecified.

A string literal expression consists of one or more consecutive string tokens. The value of the string
literal is the concatenation of the string token values. Consecutive string tokens may be separated by
whitespace. Unlike most other grammar productions, whitespace within a string token is significant and
forms part of the string value.

A string token has two forms:

1. Double-quoted string (*`DQuotedString`*)
2. Raw string (*`RawString`*)

The double-quoted string token starts with a double-quote (`"`), followed by any number of *`DStringChar`*
elements, and ends with a double-quote.

A *`DStringChar`* element encodes a single character within a double-quoted string token. For most characters,
the character is encoded as is. The exceptions are the newline character (ASCII character 10), the carriage
return character (ASCII character 13), the backslash (`\`), and the double-quote character (`"`). These
characters are always encoded using an escape sequence.

An escape sequence begins with a backslash (`\`). The following table describes the escape sequences and their
respective character values:

Escape sequence                    | Encoded character value
---------------------------------- | --------------------------------------
`\'`                               | Character `'`
`\"`                               | Character `"`
`\\`                               | Character `\`
`\?`                               | Character `?`
`\a`                               | ASCII character 7 (bell)
`\b`                               | ASCII character 8 (backspace)
`\f`                               | ASCII character 12 (form feed)
`\n`                               | ASCII character 10 (newline)
`\r`                               | ASCII character 13 (carriage return)
`\t`                               | ASCII character 9 (horizontal tab)
`\v`                               | ASCII character 11 (vertical tab)
`\`<em>nnn</em>                    | Octal number specifying an 8-bit character code (1-3 digits)
`\x`<em>nnn</em>                   | Character code in hexadecimal format (one or more digits)
`\x{`<em>nnn</em>`}`               | Character code in hexadecimal format (one or more digits)

The octal and hexadecimal numbers in escape sequences must be in the range 0–255.

A raw string starts with **`'R"'`**, followed by a user-defined delimiter *`RawStringDelim`* and **`'('`**.
The character sequence *`RawStringContent`* that follows is taken verbatim — no escape processing is performed
— and may contain any sequence of characters that does not include the termination sequence. The raw string
terminates with **`')'`** followed by the same *`RawStringDelim`* and the closing double quote **`'"'`**.

**Examples:**

```hlsl
""                      // empty string
"123"                   // string "123"
"some\nstring"          // "some" and ASCII 10 (newline) and "string"
"a \"quoted\" string"   // a "quoted" string
"contains a ' quote"    // single quote needs no escape in a double-quoted string
"\110\145\154\154\157"  // "Hello"
"\x48\x65\x6C\x6C\x6F"  // "Hello"
"\x{41}BC"              // "ABC"
"\0"                    // String containing only the null character.
R"(Raw " string)"       // value: Raw " string
R"xz(Raw " string)xz"   // value: Raw " string

R"a(Raw )a" string)a"   // The raw string parses as R"a(Raw )a"; its content is
                        // "Raw " (note the trailing space). The leftover
                        // ' string)a"' is a syntax error.

R"(ABC
DEF)"                   // value: ABC, a newline, then DEF

"123" "456"             // "123456"
```

> 📝 **Remark 1:** The recommended encoding of a string literal is UTF-8. This is not enforced.

> 📝 **Remark 2:** The current implementation does not fully conform to the language manual.
> This is tracked by GitHub issues [#11291](https://github.com/shader-slang/slang/issues/11291)
> and [#11306](https://github.com/shader-slang/slang/issues/11306).


## Character Literal Expressions

> *`CharLiteral`* = **`<'>`** *`SChar`* **`<'>`**
>
> *`SChar`* = <br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`SCharUnquoted`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuoted`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuotedOctal`* \|<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`DStringCharQuotedHex`*
>
> *`SCharUnquoted`* = **`<[^\'[:newline:]]>`**
>
> Note: [:newline:] consists of characters `\r` and `\n`. (See the escape sequence table below.)

A character literal expression evaluates to a single character value. The type of the value is
[uint](types-fundamental.md#integer). The character literal consists of an *`SChar`* enclosed in single quotes
(`'`). *`SChar`* follows the same rules as *`DStringChar`* in a double-quoted string, except that an unquoted
character may be a double quote (`"`) but may not be a single quote (`'`). A single quote must be escaped as
`\'`.

The hexadecimal numbers in escape sequences must be in the range 0–4294967295 (i.e.,
representable as `uint`). The octal escapes are limited to 0–255.

```hlsl
'\0'                    // Character 0 (null character)
'A'                     // Character 65 (A)
'\t'                    // Character 9 (horizontal tab)
'\x53'                  // Character 83 (S)
'"'                     // Character 34 (") -- double quote needs no escape
'\''                    // Character 39 (') -- single quote must be escaped
'\\'                    // Character 92 (\)
'\110'                  // Character 72 (H) via octal escape
'\x{75bcd15}'           // Character 123456789 via hexadecimal escape
```

> 📝 **Remark 1:** The current implementation does not fully conform to the language manual.
> This is tracked by GitHub issues [#11291](https://github.com/shader-slang/slang/issues/11291)
> and [#11306](https://github.com/shader-slang/slang/issues/11306).
