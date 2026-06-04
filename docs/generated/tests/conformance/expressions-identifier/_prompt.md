# Prompt: docs/generated/tests/conformance/expressions-identifier/

See [`_common.md`](_common.md).

## Target

[`docs/language-reference/expressions-identifier.md`](../../../../language-reference/expressions-identifier.md).

## Claim-extraction strategy

The doc is short (~28 lines) with two named sections: **Overloading** and **Implicit Lookup**.
Plus a brief preamble on identifier expression semantics (value lookup and l-value status).

Every normative sentence in the doc is a claim. Non-normative lines are the opening title,
the code block showing `someName`, and the section headings themselves. All remaining prose
is normative and testable.

### Preamble claims (no heading)

- C1: An identifier expression looks up the name in the current environment and yields the
  value of the matching declaration. (Observable: reading a local or module-scope name works.)
- C2: An identifier expression is an l-value when the declaration it refers to is mutable.
  (Observable: direct assignment and `inout` passing both succeed for a mutable local.)

### Overloading section

- C3: An identifier expression can be overloaded — it may refer to one or more candidate
  declarations with the same name.
- C4: When the correct candidate can be disambiguated from context, that candidate is used.
  (Observable: different argument types at a call site select different overloads.)
- C5: When an overloaded name cannot be disambiguated, its use is an error at the use site.
  (Observable: `E39999` is emitted when two overloads are equally applicable.)

### Implicit Lookup section

- C6: In the body of a method, a bare identifier `someName` may resolve to `this.someName`
  via the implicit `this` parameter.
  (Observable: a struct method and an extension method can read/write fields by bare name.)
- C7: When a global-scope `cbuffer` or `tbuffer` declaration is used, a bare identifier may
  refer to a field declared inside the `cbuffer` or `tbuffer`.
  (Observable: the emitted HLSL qualifies the field access through the cbuffer/tbuffer type.)

## What NOT to test

- The l-value claim for immutable declarations (e.g. `const` local) is the negative side of C2,
  but `const`-assignment rejection is covered in the value-categories bundle; avoid duplication.
- Overload resolution tie-breaking beyond what the doc states — don't invent ranking rules
  the doc doesn't commit to.
- Implicit lookup via `interface` default method bodies — the doc does not name that surface.
- `cbuffer`/`tbuffer` field lookup inside a method (nested scope) — the doc limits this claim
  to "a reference to `someName`" without specifying scope depth; don't over-specify.
