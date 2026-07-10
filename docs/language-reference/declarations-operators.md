> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

# Operator Overload Declarations

The following operators are overloadable:

| Operator 	| Operator function                   | Canonical description                        |
|-----------|-------------------------------------|----------------------------------------------|
| `+`		| `__prefix R operator + (T)`         | identity (unary plus)                        |
| `-`		| `__prefix R operator - (T)`         | arithmetic negation (unary minus)            |
| `~` 		| `__prefix R operator ~ (T)`         | bitwise NOT (flip bits)                      |
| `!`		| `__prefix R operator ! (T)`         | Boolean negation                             |
| `++`		| `__prefix R operator ++ (inout T)`  | increment in place, return incremented value |
| `--`		| `__prefix R operator -- (inout T)`  | decrement in place, return decremented value |
| `*`		| `__prefix R operator * (T)`         | pointer dereference (experimental)           |
| `&`		| `__prefix R operator & (__ref T)`   | address of (experimental)                    |
| `++`		| `__postfix R operator ++ (inout T)` | increment in place, return value before increment |
| `--`		| `__postfix R operator -- (inout T)` | decrement in place, return value before decrement |
| `*`       | `R operator * (T1, T2)`             | multiplication                               |
| `/`       | `R operator / (T1, T2)`             | division                                     |
| `%`       | `R operator % (T1, T2)`             | remainder                                    |
| `+`       | `R operator + (T1, T2)`             | addition                                     |
| `-`       | `R operator - (T1, T2)`             | subtraction                                  |
| `<<`      | `R operator << (T1, T2)`            | bitwise left shift                           |
| `>>`      | `R operator >> (T1, T2)`            | bitwise right shift                          |
| `<`       | `R operator < (T1, T2)`             | less-than comparison                         |
| `<=`      | `R operator <= (T1, T2)`            | less-than-or-equal-to comparison             |
| `>`       | `R operator > (T1, T2)`             | greater-than comparison                      |
| `>=`      | `R operator >= (T1, T2)`            | greater-than-or-equal-to comparison          |
| `==`      | `R operator == (T1, T2)`            | equal-to comparison                          |
| `!=`      | `R operator != (T1, T2)`            | not-equal-to comparison                      |
| `&`       | `R operator & (T1, T2)`             | bitwise AND                                  |
| `^`       | `R operator ^ (T1, T2)`             | bitwise XOR                                  |
| `|`       | `R operator | (T1, T2)`             | bitwise OR                                   |
| `&&`      | `R operator && (T1, T2)`            | logical AND                                  |
| `||`      | `R operator || (T1, T2)`            | logical OR                                   |
| `=`       | `R operator = (inout T1, T2)`       | assignment                                   |
| `+=`      | `R operator += (inout T1, T2)`      | compound addition and assignment             |
| `-=`      | `R operator -= (inout T1, T2)`      | compound subtraction and assignment          |
| `*=`      | `R operator *= (inout T1, T2)`      | compound multiplication and assignment       |
| `/=`      | `R operator /= (inout T1, T2)`      | compound division and assignment             |
| `%=`      | `R operator %= (inout T1, T2)`      | compound remainder and assignment            |
| `<<=`     | `R operator <<= (inout T1, T2)`     | compound bitwise left shift and assignment   |
| `>>=`     | `R operator >>= (inout T1, T2)`     | compound bitwise right shift and assignment  |
| `&=`      | `R operator &= (inout T1, T2)`      | compound bitwise AND shift and assignment    |
| `^=`      | `R operator ^= (inout T1, T2)`      | compound bitwise XOR and assignment          |
| `|=`      | `R operator |= (inout T1, T2)`      | compound bitwise OR and assignment           |
| `,`       | `R operator , (T1, T2)`             | comma operator (Slang 2025 and earlier)      |

In addition, call, subscript, and property access operators may be declared for structs. See
[Structures](types-struct.md) for details.

> 📝 **Remark:** Starting from Slang 2026, the comma (`,`) is no longer an operator. It is used as a separator
> in [call expressions](expressions-operators.md), [initializer expressions](expressions-initializer.md),
> [tuple expressions](types-tuple.md), and various [declarations](declarations.md).
