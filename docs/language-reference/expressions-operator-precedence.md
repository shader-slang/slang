Operator Precedence
===================

Operator precedence determines how subexpressions are grouped. Operators with higher precedence (a lower
numeric level) bind more tightly to their operands. When operators share the same level, associativity
determines the order in which they group.

| Level | Operators                                                                  | Associativity |
| ----- | -------------------------------------------------------------------------- | ------------- |
| 0     | atoms (literals, names, parenthesized, builtin keyword expressions)        | —             |
| 1     | postfix `()` `[]` `.` `::` `->` `++` `--` `<...>` (generic specialization) | left          |
| 2     | prefix/unary `+` `-` `!` `~` `++` `--` `*` `&`                             | right         |
| 3     | `*` `/` `%`                                                                | left          |
| 4     | `+` `-`                                                                    | left          |
| 5     | `<<` `>>`                                                                  | left          |
| 6     | `<` `<=` `>` `>=`                                                          | left          |
| 7     | `==` `!=`                                                                  | left          |
| 8     | `&`                                                                        | left          |
| 9     | `^`                                                                        | left          |
| 10    | `\|`                                                                       | left          |
| 11    | `&&`                                                                       | left          |
| 12    | `\|\|`                                                                     | left          |
| 13    | `?:` (ternary)                                                             | right         |
| 14    | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `\|=` `^=`                   | right         |
| 15    | `,` (Slang 2025 and earlier)                                               | left          |

Starting in Slang 2026, the comma `,` is no longer an expression operator. Instead, it serves as a separator
in grammatical constructs. This change was made to enable the new tuple syntax in Slang 2026.


## Examples

**Different precedence levels:**

```hlsl
a + b * c
// <==>
a + (b * c)      // `*` has higher precedence than `+`

a + b * c == d && e << f
// <==>
((a + (b * c)) == d) && (e << f)
```

**Same precedence levels:**

```hlsl
a + b + c + d
// <==>
((a + b) + c) + d    // left associativity

a = b = c
// <==>
a = (b = c)          // right associativity

!!a
// <==>
!(!a)                // right associativity

++ ++a
// <==>
++(++a)              // right associativity
```

**Mixed precedence and associativity:**
```hlsl
a = b = c + d + e
// <==>
a = (b = ((c + d) + e)) // `=` is right-associative
                        //
                        // `+` is left-associative
                        //     with higher precedence
```

**Explicit subexpression grouping:**
```hlsl
(a + b) * c  // parenthesized subexpressions have highest precedence
```
