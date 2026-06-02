# Prompt: docs/generated/tests/spec/expressions-operators/

See [`_common.md`](_common.md) for universal rules. Read
`### Source-of-truth hierarchy` first.

## Target

Bundle at
`docs/generated/tests/spec/expressions-operators/`,
anchored to
[`docs/language-reference/expressions-operators.md`](../../../../language-reference/expressions-operators.md).

The doc is marked TODO at the top but the operator tables and the
prefix/postfix/conditional semantics are normative content. Test
those.

## Claims with mandatory coverage

1. **Prefix `+x` is identity** — value unchanged.
2. **Prefix `-x` is arithmetic negation** — sign flipped; check the
   smallest-int boundary as a corner case.
3. **Prefix `~x` is bitwise negation** — all bits flipped.
4. **Prefix `!x` is boolean negation** — true ↔ false.
5. **Prefix `++x` yields the NEW value** (per the doc's bullet list:
   "Read → Increment → Write → Yield the new value"). Distinguishable
   from postfix only via the yield-old-vs-new contract.
6. **Postfix `x++` yields the OLD value** (per the doc's bullet list:
   "Read → Increment → Write → Yield the old value").
7. **`?:` does NOT short-circuit** — explicit doc note: "Unlike C,
   C++, GLSL, and most other C-family languages, Slang currently
   follows the precedent of HLSL where `?:` does not short-circuit."
   Verify by placing a side-effect on the unselected branch and
   observing that it ran.
8. **`?:` works component-wise on `bool` vectors** — when the
   condition is a vector, both alternatives must be vectors and
   selection is per-component.
9. **Parenthesized expression** — same value as the wrapped
   expression.
10. **Compatibility cast** `(MyStruct)0` — equivalent to `{}` init.

## What NOT to test here

- Operator overloading on user-defined types — the doc references
  this but the overload resolution rules belong in name-resolution
  bundles.
- Short-circuit `&&` / `||` — these are documented in the doc but
  also exercised at the slangi-VM-bug intersection in the
  `ast-reference/expressions` bundle. Avoid duplicate coverage; the
  doc's claim that they short-circuit (in contrast to `?:`) can be
  cross-referenced rather than re-tested here.

## Observation tooling

- Value claims: `//TEST:INTERPRET(filecheck=CHECK):` and printf.
- Use the **function-param pattern** to defeat the constant folder
  for arithmetic claims.
- For the `?:`-doesn't-short-circuit claim, use a function that
  increments a counter and check the counter is bumped even when
  the branch is "unselected".
