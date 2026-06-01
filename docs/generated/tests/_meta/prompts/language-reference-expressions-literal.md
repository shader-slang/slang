# Prompt: docs/generated/tests/language-reference/expressions-literal/

See [`_common.md`](_common.md) for universal rules. **Read the
`### Source-of-truth hierarchy` section in `_common.md` first.** The
language-reference subtree exists because the language reference
manual is the authoritative spec, outranking the generated design
docs. Every test in this bundle must anchor to a claim made in the
language reference itself, not the design docs.

## Target

Produce the test bundle at
`docs/generated/tests/language-reference/expressions-literal/`,
anchored to
[`docs/language-reference/expressions-literal.md`](../../../../language-reference/expressions-literal.md).

The doc specifies the **grammar** and **semantics** of literal
expressions: boolean, integer (decimal / hexadecimal / binary /
octal — the last is deprecated), floating-point, string, and the
special `nullptr` literal. It also fixes the **suffix → type**
table and the smallest-negative-integer exception.

Audience: nightly CI. Tests exercise the lexer + checker. No GPU
required.

## The translation rule

Each grammar production or table row in the language reference is a
testable claim. The translation rule is:

- **Testable** ⇔ "if the doc's claim were false, the program-text
  behaviour `slangc` produces would change in a way a `//TEST` or
  `//DIAGNOSTIC_TEST` directive can observe."
- **Not testable through slangc** ⇔ "the claim is about the lexer's
  internal tokenization, the AST node class allocated, or a parser
  helper invoked — none of which surface at the CLI."

The doc is **dense with mandatory boundary axes** per
[`_common.md § Mandatory axes`](_common.md). Cover them.

### Claims with mandatory boundary coverage

The doc enumerates concrete, testable claims:

1. **Decimal literals.** `0` alone is decimal; non-zero leading digit
   followed by zero or more decimal digits. Anchor:
   `expressions-literal.md#integer-literal-expressions`.

2. **Hexadecimal literals.** Prefix `0x` or `0X` (case-insensitive),
   followed by one or more hex digits `0-9A-Fa-f`. The body length
   determines the value; the suffix or value range determines the
   type per the table.

3. **Binary literals.** Prefix `0b` or `0B`, followed by one or more
   `0`/`1` digits.

4. **Octal literals (deprecated).** Leading `0` followed by one or
   more `0-7` digits. **Deprecated — triggers a warning**, supported
   only for backwards compatibility. Note: `0` alone is decimal,
   `00`, `01`, etc. are octal.

5. **Smallest-negative-integer exceptions.** Three explicit rules in
   the doc that let `-2147483648` and `-9223372036854775808` be
   spelled without an immediate overflow promotion:
   - `2147483648` or `2147483648L` preceded by unary minus →
     `int(-2147483648)`.
   - `9223372036854775808`, `9223372036854775808L`, or
     `9223372036854775808LL` preceded by unary minus →
     `int64_t(-9223372036854775808)`.
   - `2147483648Z` (32-bit pointers) or `9223372036854775808Z`
     (64-bit pointers) preceded by unary minus →
     `intptr_t(...)`.

6. **Suffix → type table.** The doc fixes the type-selection rules.
   Of particular note (asterisked rows trigger a warning):
   - `(none)/L` on a decimal value in `[2147483648, 9223372036854775807]`
     promotes to `int64_t`.
   - `(none)/L` on a decimal value in `[9223372036854775808, …]`
     promotes to `uint64_t` **with a warning**.
   - `U`/`UL` always lands in the unsigned column.
   - `LL` selects `int64_t` first, then `uint64_t`.
   - `Z`/`UZ`/`ZU` are pointer-width-indexed and target-dependent.

7. **Floating-point literal grammar.** Refer to the same doc's
   `#floating-point-literal-expressions` section if it exists in your
   `source_commit`. (The doc may be incomplete here — record a
   doc-gap row if the production is missing.)

8. **Boolean literals.** `true` / `false` are the only spellings.

### Observation tooling

- For **value-level claims** (smallest-negative-int, suffix-selected
  type), use `//TEST:INTERPRET(filecheck=CHECK):` and a `printf` to
  observe the parsed value. See
  [`_common.md § slangi / INTERPRET quirks`](_common.md) — and the
  known slangi VM constants-OOB issue ([#11375](https://github.com/shader-slang/slang/issues/11375))
  has affected some printf-after-side-effect tests in the past; if a
  test seems to be hitting that bug rather than the literal it is
  exercising, simplify the test (no `inout` chain, separate printfs)
  before adding to `expected-failures.txt`.
- For **type-level claims** (literal `X` has type `T`), one robust
  pattern is to assign the literal to a variable of the expected
  type without conversion and check that the SPIR-V or HLSL emit
  carries the right `TypeInt`/`TypeFloat` width. Alternatively use
  the `__check_type<T>(value)` idiom if the bundle is allowed to
  invoke compile-time helpers (see siblings under
  `ast-reference/types`).
- For **deprecation warnings** (octal), use `//DIAGNOSTIC_TEST` with
  a diagnostic-code CHECK or a position-anchored CHECK at the literal.
- For **errors** (e.g. `08` — invalid octal digit), use
  `//DIAGNOSTIC_TEST` and pin the diagnostic.

### What NOT to test

- AST-level claims (`IntegerLiteralExpr` is a subclass of
  `LiteralExpr`) — out of scope; the AST shape is not a CLI surface.
- Lexer-internal claims about how the regex backtracks — out of
  scope.
- Behaviours the language reference does **not** describe — record
  a doc-gap, do not write a test against your own intuition.

### Cross-target coverage

Most claims are target-independent (they hold at parse/check time).
Use one `//TEST:INTERPRET` or `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`
per claim. For claims that touch emit (e.g. `int64_t` vs `uint64_t`
emit), also add `-target spirv-asm` so the SPIR-V `TypeInt` width
appears in the CHECK.

## Bundle size guidance

Target ~25-35 tests. The doc has many sub-claims; cover the boundary
axes the doc itself enumerates (every row of the suffix→type table,
every smallest-negative-int rule, every grammar production for
integer literals) and skip floating-point literals if the doc
section is incomplete (record as a doc-gap).
