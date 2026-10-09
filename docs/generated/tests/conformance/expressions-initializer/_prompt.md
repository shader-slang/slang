# Prompt: docs/generated/tests/conformance/expressions-initializer/

See [`_common.md`](../_common.md) for universal rules. Read
`### Source-of-truth hierarchy` first.

## Target

Bundle at
`docs/generated/tests/conformance/expressions-initializer/`,
anchored to
[`docs/language-reference/expressions-initializer.md`](../../../../language-reference/expressions-initializer.md).

The doc is marked `> TODO` at the top, but the body below that marker is
normative prose describing two distinct language features with testable
behavioural claims. Test those.

## Claim extraction strategy

The doc is small (~35 lines). All claims can be extracted by reading
the two named sections verbatim:

### Section: "Initializer Expressions" (`### Initializer Expressions`)

Sentence-level claims:

- **C1** — "When the base expression of a call is a type instead of a
  value, the expression is an initializer expression." (Definition /
  normative rule.) Observable by checking that `T(args...)` is accepted
  and produces a value of type `T`.
- **C2** — "An initializer expression initialized an instance of the
  specified type using the given arguments." (Semantic rule, closely
  related to C1 but separately stated — the arguments actually feed
  the fields/parameters.) Observable by verifying the resulting field
  values.
- **C3** — "An initializer expression with only a single argument is
  treated as a cast expression." Supported by the doc example showing
  `int(1.0f)` ≡ `(int) 1.0f`. Observable by checking both produce the
  same runtime value.

C1 and C2 are tightly coupled (type-as-callee → construction) and can
share a functional test; they are counted as two claims because the
first is definitional and the second is the semantic consequence.

### Section: "Initializer List Expression" (underline-style heading)

Sentence-level claims:

- **C4** — "An _initializer list expression_ comprises zero or more
  expressions, separated by commas, enclosed in `{}`." Observable by
  testing a zero-element list (`{}`), a one-element list, and a
  multi-element list as the initial-value of a variable.
- **C5** — "An initialier list expression may only be used directly as
  the initial-value expression of a variable or parameter declaration;
  initializer lists are not allowed as arbitrary sub-expressions."
  In practice the compiler is **more permissive** than this claim (it
  also accepts init lists in function-call arguments, return
  expressions, and assignment). This discrepancy is recorded as a
  doc gap. A positive test for the permitted use (variable declaration
  init) is still written for C4; the restriction claim is recorded as
  a doc gap rather than a negative test that would fail against the
  compiler's actual behaviour.

### What NOT to test here

- Detailed rules for how each kind of type initializes from an
  initializer list (the doc itself marks this as a future TODO).
- Operator overloading / `__init` constructors — those are covered in
  type-specific bundles.
- `cast` expressions as a standalone feature — they share the C3 test
  via equivalence, not as a separate bundle.

## Observation tooling

- Value claims: `//TEST:INTERPRET(filecheck=CHECK):` + `printf`.
- Emission claims: `//TEST:SIMPLE(filecheck=CHECK):-target hlsl` paired
  with functional tests.
- Use function-parameter indirection to defeat the constant folder.
- For C3 (cast equivalence), pass the float through a function
  parameter so the compiler cannot fold the conversion away before the
  functional test observes it.
