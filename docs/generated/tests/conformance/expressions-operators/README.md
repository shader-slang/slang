---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T14:00:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 7662e1d58793f553dee236318be8cecbf5546ee863fccd71a0bd767a68e6c6d5
source_doc: docs/language-reference/expressions-operators.md
source_doc_digest: 7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/expressions-operators

## Intent

Tests verify operator semantics claimed in the **language reference**
at
[`docs/language-reference/expressions-operators.md`](../../../../language-reference/expressions-operators.md).
The doc enumerates prefix / postfix / infix operator tables plus
explicit rules for the conditional `?:` (HLSL-style non-short-
circuiting), parenthesized, and cast expressions.

The high-value claims (most likely to find compiler bugs):

- The **prefix vs postfix yield-old-vs-new** distinction is a
  classic source of bugs. The doc states the semantics in explicit
  bullet form.
- **`?:` does not short-circuit** is a Slang-specific carve-out
  versus C-family languages. Tests pin this with a side-effect on
  the unselected branch.

## Functional coverage

| Claim                                                                                                                                      | Intent     | Anchor                                                                                                                | Tests                                                                                  |
| ------------------------------------------------------------------------------------------------------------------------------------------ | ---------- | --------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| Prefix `+` is identity: `+x == x`.                                                                                                         | functional | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions)   | [`prefix-plus-identity.slang`](prefix-plus-identity.slang)                             |
| Prefix `-` is arithmetic negation: `-x == 0 - x`.                                                                                          | functional | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions)   | [`prefix-minus-negation.slang`](prefix-minus-negation.slang)                           |
| Prefix `~` is bitwise negation: `~x` flips all bits.                                                                                       | functional | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions)   | [`prefix-tilde-bitwise.slang`](prefix-tilde-bitwise.slang)                             |
| Prefix `!` is Boolean negation: `!true == false`, `!false == true`.                                                                        | functional | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions)   | [`prefix-bang-bool-negation.slang`](prefix-bang-bool-negation.slang)                   |
| Prefix `++` yields the **new** value (post-increment).                                                                                     | boundary   | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions)   | [`prefix-increment-yields-new.slang`](prefix-increment-yields-new.slang)               |
| Postfix `++` yields the **old** value.                                                                                                     | boundary   | [#postfix-operator-expressions](../../../../language-reference/expressions-operators.md#postfix-operator-expressions) | [`postfix-increment-yields-old.slang`](postfix-increment-yields-old.slang)             |
| Prefix `--` yields the new (decremented) value; postfix `--` yields the old value.                                                         | boundary   | [#postfix-operator-expressions](../../../../language-reference/expressions-operators.md#postfix-operator-expressions) | [`prefix-vs-postfix-decrement.slang`](prefix-vs-postfix-decrement.slang)               |
| Conditional `?:` does **not** short-circuit (HLSL semantics, contra C-family). Both branches evaluate; only the selected value is yielded. | boundary   | [#conditional-expression](../../../../language-reference/expressions-operators.md#conditional-expression)             | [`conditional-does-not-short-circuit.slang`](conditional-does-not-short-circuit.slang) |
| Parenthesized expression has the same value as the wrapped expression.                                                                     | functional | [#parenthesized-expression](../../../../language-reference/expressions-operators.md#parenthesized-expression)         | [`parens-same-value.slang`](parens-same-value.slang)                                   |
| Compatibility cast `(StructTy)0` is equivalent to `{}` initialization for user-defined structs.                                            | functional | [#compatibility-feature](../../../../language-reference/expressions-operators.md#compatibility-feature)               | [`cast-zero-to-struct.slang`](cast-zero-to-struct.slang)                               |

## Untested claims

| Claim                                                                                                                                                       | Reason        | Anchor                                                                                                              | Why untested                                                                                                                                                                                                                                      |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- | ------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The full infix-operator table — every row of multiplicative / additive / shift / relational / equality / bitwise / logical / compound-assignment operators. | out-of-bundle | [#infix-operator-expressions](../../../../language-reference/expressions-operators.md#infix-operator-expressions)   | The standard hand-written `tests/` suite exercises these heavily already; spec-anchored boundary coverage here would mostly duplicate. The prefix/postfix and `?:` claims above are the high-leverage targets that the broader suite under-tests. |
| Operator overloading via `operator+(...)` and `__prefix` / `__postfix` keywords.                                                                            | out-of-bundle | [#prefix-operator-expressions](../../../../language-reference/expressions-operators.md#prefix-operator-expressions) | The overload-resolution semantics live in a name-resolution bundle.                                                                                                                                                                               |
| Call expression and subscript expression sections.                                                                                                          | out-of-bundle | [#call-expression](../../../../language-reference/expressions-operators.md#call-expression)                         | These claims are exercised in `ast-reference/expressions`.                                                                                                                                                                                        |

## Doc gaps observed

NA
