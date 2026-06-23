# Prompt: docs/generated/tests/conformance/expressions-operator-precedence/

See [`_common.md`](../../../_meta/prompts/_common.md) for universal rules. Read
`### Source-of-truth hierarchy` first.

## Target

Bundle at
`docs/generated/tests/conformance/expressions-operator-precedence/`,
anchored to
[`docs/language-reference/expressions-operator-precedence.md`](../../../../language-reference/expressions-operator-precedence.md).

The doc is a single precedence table (16 levels × 3 columns: Level,
Operators, Associativity) followed by worked examples. Every row of
the table and every worked example is normative.

## Sub-areas

### 1. The precedence table

Each row is a claim: the set of operators at that level, and the
associativity for that level. There are 16 rows (levels 0–15).

Level 0 (atoms) is non-testable via arithmetic — it is the grammar
production for primary expressions. Level 15 (`,`) is marked
"Slang 2025 and earlier" with a language change note for 2026.

Rows 1–14 each yield a **precedence claim** (relative tightness
against adjacent levels) and an **associativity claim** (left or
right). The natural test for a precedence claim is an expression
where the two possible groupings yield **different observable values**,
e.g. `2 + 3 * 4 == 14` (not 20).

### 2. Adjacent-level precedence pairs

The key pairs to test (each tighter level listed first):

- Level 3 `*/%` > Level 4 `+-` — canonical case
- Level 4 `+-` > Level 5 `<< >>` — less intuitive
- Level 5 `<< >>` > Level 6 relational
- Level 6 relational > Level 7 `== !=`
- Level 7 `== !=` > Level 8 `&` (bitwise)
- Level 8 `&` > Level 9 `^`
- Level 9 `^` > Level 10 `|`
- Level 10 `|` > Level 11 `&&`
- Level 11 `&&` > Level 12 `||`
- Level 12 `||` > Level 13 `?:`
- Level 13 `?:` > Level 14 `=` and compound assignments
- Level 1 postfix > Level 2 prefix/unary

### 3. Associativity claims per level

The table explicitly names associativity for each level:

- Level 1 (postfix): left
- Level 2 (prefix/unary): **right** (doc example: `!!a` = `!(!a)`)
- Level 3 `*/%`: left
- Level 4 `+-`: left (doc example: `a+b+c+d` = `((a+b)+c)+d`)
- Level 5 `<< >>`: left
- Level 6 relational: left
- Level 7 `== !=`: left
- Level 8 `&`: left
- Level 9 `^`: left
- Level 10 `|`: left
- Level 11 `&&`: left
- Level 12 `||`: left
- Level 13 `?:`: **right** (doc example: chained ternary)
- Level 14 `=` and compounds: **right** (doc examples: `a=b=c`, `a=b=c+d+e`)
- Level 15 `,`: left (Slang 2025 and earlier)

### 4. The Slang-2026 comma change

The doc states: "Starting in Slang 2026, the comma `,` is no longer
an expression operator. Instead, it serves as a separator in
grammatical constructs." This is a language-change note, not an
operator claim for 2026 usage. No runtime test can distinguish
the two uses without version-gated compilation. Record as untested
with reason `implementation-detail` (no slang-test directive selects
by language version).

### 5. Parenthesized subexpressions

Level 0 includes "parenthesized" expressions. The doc example
`(a + b) * c` shows parentheses overriding the natural `*`-before-`+`
grouping. Test as `(2+3)*4 = 20` versus `2+3*4 = 14`.

### 6. The doc's worked examples (normative)

The doc provides four example blocks, each with an equivalence:

1. `a + b * c == d && e << f` — multi-level grouping
2. `a + b + c + d` left-assoc and `a = b = c` right-assoc
3. `!!a` and `++ ++a` — right-assoc unary
4. `a = b = c + d + e` — mixed right-assoc `=` with left-assoc `+`
5. `(a + b) * c` — explicit parenthesization

All five are normative and directly testable.

## Claim-extraction strategy

- Each table row → one "precedence vs. next-lower level" claim +
  one "associativity" claim.
- Level 0 (atoms) → associativity is `—` (not applicable); skip.
- Level 15 (`,` Slang 2025) → version-gated; untested with reason.
- Worked examples → one test per example (may combine claims from
  the same example into one test file).
- For associativity claims where both orderings yield the **same
  value** by coincidence, choose operand values carefully so the two
  groupings differ (e.g. for left-assoc `/`: `16/4/2=2` vs
  `16/(4/2)=8`).
- Use `int` parameters (not `bool`) in helper functions to avoid
  the slangi bool-parameter-passing quirk.

## What NOT to test here

- The semantics of each operator (what `+` does, what `<<` does) —
  those belong in the `expressions-operators` bundle.
- Short-circuit behavior of `&&`/`||` — the doc does not claim
  these short-circuit; the `?:` non-short-circuit behavior is in
  the `expressions-operators` bundle.
- Operator overloading for user-defined types.
- Generic specialization `<...>` (level 1 postfix) — this requires
  type-system machinery beyond a simple precedence test.

## Observation tooling

- Value claims: `//TEST:INTERPRET(filecheck=CHECK):` with printf.
- Use the **function-param pattern** (`int foo(int a, int b)`)
  to defeat the constant folder. Always use `int` parameters, not
  `bool`, to avoid the slangi bool-parameter-passing quirk.
- COMPUTE-dispatch backup: `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`
  with `//TEST_INPUT: set outputBuffer = out ubuffer(data=[...], stride=4)`.
- Emission: `//TEST:SIMPLE(filecheck=CHECK):-target hlsl` when pinning
  emitted expression structure.
