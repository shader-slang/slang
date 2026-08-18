---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T09:20:02+00:00
source_commit: 54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
watched_paths_digest: fdd1d7d4de28d99fc0a6516aba6f28447037f9a4aad2516c8b9c18936773972f
source_doc: docs/language-reference/expressions-operator-precedence.md
source_doc_digest: bf53e1be8859db88ce191d1088c4bc28cb3b6ab91aac3512925d33244244bed5
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/expressions-operator-precedence

## Intent

Tests verify operator precedence and associativity claims in the **language reference**
at [`docs/language-reference/expressions-operator-precedence.md`](../../../../language-reference/expressions-operator-precedence.md).
The doc is a 16-row table (levels 0–15) plus four worked-example blocks. Every row
contributes a precedence claim (tighter than adjacent lower level) and an associativity
claim. The strategy is to choose operand values where the two possible groupings yield
**different observable values**, making the test self-distinguishing: a wrong grouping
reports a different number, not just "wrong" with no signal.

## Claims

### Precedence table: operator level per row

- C1. Level 0 — atoms (literals, names, parenthesized, builtin keyword expressions) are the highest-precedence expressions; associativity is not applicable.
- C2. Level 1 — postfix `()`, `[]`, `.`, `::`, `->`, `++`, `--`, `<...>` (generic specialization) are left-associative.
- C3. Level 2 — prefix/unary `+`, `-`, `!`, `~`, `++`, `--`, `*`, `&` are right-associative.
- C4. Level 3 — `*`, `/`, `%` are left-associative.
- C5. Level 4 — `+`, `-` are left-associative.
- C6. Level 5 — `<<`, `>>` are left-associative.
- C7. Level 6 — `<`, `<=`, `>`, `>=` are left-associative.
- C8. Level 7 — `==`, `!=` are left-associative.
- C9. Level 8 — `&` (bitwise and) is left-associative.
- C10. Level 9 — `^` (bitwise xor) is left-associative.
- C11. Level 10 — `|` (bitwise or) is left-associative.
- C12. Level 11 — `&&` (logical and) is left-associative.
- C13. Level 12 — `||` (logical or) is left-associative.
- C14. Level 13 — `?:` (ternary) is right-associative.
- C15. Level 14 — `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=`, `&=`, `|=`, `^=` are right-associative.
- C16. Level 15 — `,` is left-associative (Slang 2025 and earlier).

### Relative precedence between adjacent levels

- C17. Level 3 (`*`) binds more tightly than level 4 (`+`): `2 + 3 * 4` = 14, not 20.
- C18. Level 4 (`+`) binds more tightly than level 5 (`<<`): `2 + 3 << 1` = 10, not 8.
- C19. Level 5 (`<<`) binds more tightly than level 6 (`<`): `1 << 3 < 10` = true (8<10), not 2.
- C20. Level 6 (relational) binds more tightly than level 7 (`==`): `1 < 2 == 3 < 4` = `(1<2)==(3<4)` = true.
- C21. Level 7 (`==`) binds more tightly than level 8 (`&`): `3 & 5 == 5` = `3 & (5==5)` = `3 & 1` = 1.
- C22. Level 8 (`&`) binds more tightly than level 9 (`^`): `6 & 5 ^ 3` = `(6&5)^3` = `4^3` = 7, not 6.
- C23. Level 9 (`^`) binds more tightly than level 10 (`|`): `5 ^ 6 | 4` = `(5^6)|4` = `3|4` = 7, not 3.
- C24. Level 10 (`|`) binds more tightly than level 11 (`&&`): `4 | 0 && 0` = `(4|0)&&0` = false (0), not 1.
- C25. Level 11 (`&&`) binds more tightly than level 12 (`||`): `true || false && false` = `true||(false&&false)` = true, not false.
- C26. Level 12 (`||`) binds more tightly than level 13 (`?:`): `0||0 ? 10 : 20` = `(0||0)?10:20` = 20.
- C27. Level 13 (`?:`) binds more tightly than level 14 (`=`): chained ternary and assignment interact correctly.
- C28. Level 1 postfix binds more tightly than level 2 prefix/unary: `-arr[1]` = `-(arr[1])`, not `(-arr)[1]`.
- C29. Level 2 prefix/unary binds more tightly than level 3 (`*`): `-a + b` = `(-a)+b`, not `-(a+b)`.

### Parenthesization

- C30. Parenthesized subexpressions override natural precedence: `(a+b)*c` groups addition before multiplication.

### Worked examples from the doc

- C31. Doc example: `a + b * c == d && e << f` groups as `((a+(b*c))==d) && (e<<f)`.
- C32. Doc example: `a + b + c + d` groups as `((a+b)+c)+d` (left-assoc `+`); verified by `10-3-2=5`.
- C33. Doc example: `a = b = c` groups as `a=(b=c)` (right-assoc `=`); both get the same value.
- C34. Doc example: `!!a` groups as `!(!a)` (right-assoc prefix).
- C35. Doc example: `a = b = c + d + e` groups as `a=(b=((c+d)+e))` — right-assoc `=` with left-assoc `+`.

### Language version note

- C36. Starting in Slang 2026, `,` is no longer an expression operator; it serves as a grammatical separator (enabling tuple syntax).

## Functional coverage

| Claim                                                                                             | Intent     | Anchor                                                                                                        | Tests                                                                                                                                                                    |
| ------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C17: Level 3 `*` binds more tightly than level 4 `+`: `2 + 3 * 4` = 14, not 20.                   | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`mul-over-add.slang`](mul-over-add.slang), [`mul-over-add-emission.slang`](mul-over-add-emission.slang), [`mul-over-add-hlsl-emit.slang`](mul-over-add-hlsl-emit.slang) |
| C4 + C32: Level 3 `*`/`/`/`%` left-associative: `16/4/2` = 2 (not 8).                             | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`mul-left-assoc.slang`](mul-left-assoc.slang)                                                                                                                           |
| C5 + C32: Level 4 `+`/`-` left-associative: `10-3-2` = 5 (not 9).                                 | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`add-left-assoc.slang`](add-left-assoc.slang)                                                                                                                           |
| C18: Level 4 `+` binds more tightly than level 5 `<<`: `2+3<<1` = 10, not 8.                      | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`add-over-shift.slang`](add-over-shift.slang)                                                                                                                           |
| C6: Level 5 `<<`/`>>` left-associative: `1<<2<<1` = 8 (not 16).                                   | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`shift-left-assoc.slang`](shift-left-assoc.slang)                                                                                                                       |
| C19: Level 5 `<<` binds more tightly than level 6 `<`: `1<<3<10` = true (8<10).                   | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`shift-over-relational.slang`](shift-over-relational.slang)                                                                                                             |
| C20: Level 6 relational binds more tightly than level 7 `==`: `1<2==3<4` = `(1<2)==(3<4)` = true. | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`relational-over-equality.slang`](relational-over-equality.slang)                                                                                                       |
| C21: Level 7 `==` binds more tightly than level 8 `&`: `3 & 5==5` = `3&(5==5)` = 1.               | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`equality-over-bitand.slang`](equality-over-bitand.slang)                                                                                                               |
| C22: Level 8 `&` binds more tightly than level 9 `^`: `6&5^3` = `(6&5)^3` = 7.                    | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`bitand-over-xor.slang`](bitand-over-xor.slang)                                                                                                                         |
| C23: Level 9 `^` binds more tightly than level 10 `\|`: `5^6\|4` = `(5^6)\|4` = 7.                | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`xor-over-bitor.slang`](xor-over-bitor.slang)                                                                                                                           |
| C24: Level 10 `\|` binds more tightly than level 11 `&&`: `4\|0&&0` = `(4\|0)&&0` = false.        | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`bitor-over-logical-and.slang`](bitor-over-logical-and.slang)                                                                                                           |
| C25: Level 11 `&&` binds more tightly than level 12 `\|\|`: `true\|\|false&&false` = true.        | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`logical-and-over-logical-or.slang`](logical-and-over-logical-or.slang)                                                                                                 |
| C26: Level 12 `\|\|` binds more tightly than level 13 `?:`: `0\|\|0?10:20` = 20.                  | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`logical-or-over-ternary.slang`](logical-or-over-ternary.slang)                                                                                                         |
| C14: Level 13 `?:` is right-associative: chained ternary `a?x:b?y:z` = `a?x:(b?y:z)`.             | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`ternary-right-assoc.slang`](ternary-right-assoc.slang)                                                                                                                 |
| C15: Level 14 `=` is right-associative: `a=b=5` sets both a and b to 5.                           | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`assignment-right-assoc.slang`](assignment-right-assoc.slang)                                                                                                           |
| C15: Level 14 compound `+=` is right-associative: `a+=b=3` evaluates as `a+=(b=3)`.               | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`compound-assign-right-assoc.slang`](compound-assign-right-assoc.slang)                                                                                                 |
| C27 + C35: `?:` (level 13) binds more tightly than `=` (level 14): `a=b=c+d+e` groups correctly.  | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`ternary-over-assignment.slang`](ternary-over-assignment.slang)                                                                                                         |
| C28: Level 1 postfix `[]` binds more tightly than level 2 prefix `-`: `-arr[1]` = `-(arr[1])`.    | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`postfix-over-prefix.slang`](postfix-over-prefix.slang)                                                                                                                 |
| C3 + C29: Level 2 prefix/unary right-associative: `!!a` = `!(!a)`.                                | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`unary-right-assoc.slang`](unary-right-assoc.slang)                                                                                                                     |
| C29: Level 2 prefix unary `-` binds more tightly than level 3 `*`/`+`: `-a+b` = `(-a)+b`.         | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`unary-over-mul.slang`](unary-over-mul.slang)                                                                                                                           |
| C30: Parenthesized subexpressions override precedence: `(a+b)*c` = 20, not 14.                    | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`parens-override-precedence.slang`](parens-override-precedence.slang)                                                                                                   |
| C31: Doc example `a+b*c==d&&e<<f` groups as `((a+(b*c))==d)&&(e<<f)`.                             | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`complex-example-from-doc.slang`](complex-example-from-doc.slang)                                                                                                       |
| C35: Doc example `a=b=c+d+e` groups as `a=(b=((c+d)+e))`; with [1,2,3] both a,b = 6.              | functional | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | [`mixed-assoc-example-from-doc.slang`](mixed-assoc-example-from-doc.slang)                                                                                               |

## Untested claims

| Claim                                                                                                                                         | Reason                | Anchor                                                                                                        | Why untested                                                                                                                                                                                                                                                                 |
| --------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1: Level 0 atoms are highest-precedence expressions; associativity is not applicable.                                                        | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Level 0 is the grammar production for primary expressions. All tests implicitly rely on atoms having the highest precedence; no separate test can distinguish atoms from parenthesized atoms.                                                                                |
| C2: Level 1 postfix ops (call, subscript, member, scope, arrow, post-increment, post-decrement, generic-specialization) are left-associative. | out-of-bundle         | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Postfix subscript over prefix minus is verified in postfix-over-prefix.slang. Generic specialization left-associativity requires type-system machinery; tested in the expressions-operators bundle.                                                                          |
| C7: Level 6 relational ops (less-than, less-equal, greater-than, greater-equal) are left-associative.                                         | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Chained relational comparisons (e.g., a-less-b-less-c) are a type error in Slang (comparing bool result with int). Left-associativity of relational ops is verified implicitly by relational-over-equality.slang.                                                            |
| C8: Level 7 equality ops (equal-equal, bang-equal) are left-associative.                                                                      | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Chained equality a==b==c is a type error in Slang. Left-associativity is verified implicitly by equality-over-bitand.slang.                                                                                                                                                  |
| C9: Level 8 bitwise-and is left-associative. C10: Level 9 bitwise-xor is left-associative. C11: Level 10 bitwise-or is left-associative.      | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Associativity within a single bitwise-op level (e.g., a-and-b-and-c) yields the same result left or right when the ops are homogeneous. Relative ordering between these levels is verified by bitand-over-xor.slang, xor-over-bitor.slang, and bitor-over-logical-and.slang. |
| C12: Level 11 logical-and is left-associative.                                                                                                | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Distinguishing left vs right associativity for logical-and is difficult without side effects; left-assoc matches C. Verified implicitly by logical-and-over-logical-or.slang.                                                                                                |
| C13: Level 12 logical-or is left-associative.                                                                                                 | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Same reasoning as C12. Verified implicitly by logical-or-over-ternary.slang.                                                                                                                                                                                                 |
| C16: Level 15 comma is left-associative (Slang 2025 and earlier).                                                                             | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | No slang-test directive selects by language version (2025 vs 2026). The comma-as-operator behavior cannot be isolated from comma-as-separator without a version gate.                                                                                                        |
| C33: Doc example a=b=c right-assoc groups as a=(b=c).                                                                                         | out-of-bundle         | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Subsumed by assignment-right-assoc.slang which tests a=b=5 setting both to 5.                                                                                                                                                                                                |
| C34: Doc example !!a groups as !(!a) (right-assoc prefix).                                                                                    | out-of-bundle         | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | Covered by unary-right-assoc.slang.                                                                                                                                                                                                                                          |
| C36: Starting in Slang 2026, comma is no longer an expression operator; it serves as a grammatical separator.                                 | implementation-detail | [#operator-precedence](../../../../language-reference/expressions-operator-precedence.md#operator-precedence) | No slang-test directive selects by language version. This is a language-version change note; testing requires version-gated compilation not available in slang-test.                                                                                                         |

## Doc gaps observed

NA
