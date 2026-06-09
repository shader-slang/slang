---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T09:37:14+00:00
source_commit: 273e29cb6cac600f70da7245a506bae6689e0e23
watched_paths_digest: 0474ffc35ca34662fbe182c4c49a8d1ef9d0071c9e7b419f16894ccdc479ffc5
source_doc: docs/language-reference/expressions-this.md
source_doc_digest: 832f249cf78011a84295e7d3b91b07f24fbb276d0fa93cc94b8abec74fbf1e87
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/expressions-this

## Intent

Tests verify `this`-expression claims in the **language reference** at
[`docs/language-reference/expressions-this.md`](../../../../language-reference/expressions-this.md).
The doc is short (~16 lines, one normative section "Mutability"); the coverage strategy is
one positive + one negative test per distinct claim in that section. Claims C1 and C2 (identity
and type of `this`) are already exercised by `conformance/types-struct` and are recorded as
`out-of-bundle`. Claims C3–C5 (l-value rules for `[mutating]`, `out`/`inout`, and call
expressions) are covered here with functional and negative tests.

## Claims

### Identity and type (`#this-expression`)

- **C1** `this` refers to the implicit instance of the enclosing type being operated on in
  instance methods, subscripts, and initializers.
- **C2** The type of `this` is `This`.

### Mutability (`#mutability`)

- **C3** If a `[mutating]` method is called, the implicit `this` argument must be an l-value;
  calling it on an r-value (temporary) is rejected.
- **C4** Argument expressions corresponding to `out` or `in out` parameters must be l-values;
  passing an r-value to such a parameter is rejected.
- **C5** A call expression is never an l-value; assigning to one is rejected.

## Functional coverage

| Claim                                                                                                                                               | Intent     | Anchor                                                                       | Tests                                                                              |
| --------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ---------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| C3 — Calling a `[mutating]` member function on an l-value variable succeeds and the mutation is observable on the instance.                         | functional | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-mutating-lvalue-functional.slang`](this-mutating-lvalue-functional.slang)   |
| C3 — Calling a `[mutating]` member function on an r-value (temporary) is rejected because the implicit `this` argument must be an l-value (E30050). | negative   | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-mutating-rvalue-rejected.slang`](this-mutating-rvalue-rejected.slang)       |
| C4 — Passing an l-value variable as an `out` parameter argument succeeds and the written value is observable.                                       | functional | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-out-param-lvalue-functional.slang`](this-out-param-lvalue-functional.slang) |
| C4 — Passing an r-value expression as an `out` parameter argument is rejected because `out` arguments must be l-values (E30047).                    | negative   | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-out-param-rvalue-rejected.slang`](this-out-param-rvalue-rejected.slang)     |
| C4 — Passing an r-value expression as an `inout` parameter argument is rejected because `in out` arguments must be l-values (E30047).               | negative   | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-inout-param-rvalue-rejected.slang`](this-inout-param-rvalue-rejected.slang) |
| C5 — A call expression is never an l-value; assigning to one is rejected with E30011.                                                               | negative   | [#mutability](../../../../language-reference/expressions-this.md#mutability) | [`this-call-not-lvalue-rejected.slang`](this-call-not-lvalue-rejected.slang)       |

## Untested claims

| Claim                                                                                                                | Reason        | Anchor                                                                                 | Why untested                                                                                                                                                                          |
| -------------------------------------------------------------------------------------------------------------------- | ------------- | -------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1 — `this` refers to the implicit instance of the enclosing type in instance methods, subscripts, and initializers. | out-of-bundle | [#this-expression](../../../../language-reference/expressions-this.md#this-expression) | Covered by `conformance/types-struct`: `struct-this-implicit-optional.slang` (C26) and `struct-member-function-readonly.slang` (C27) both exercise `this` access in member functions. |
| C2 — The type of `this` is `This`.                                                                                   | out-of-bundle | [#this-expression](../../../../language-reference/expressions-this.md#this-expression) | Covered by `conformance/types-struct`: `struct-This-type-keyword.slang` (C44) verifies `This` inside a struct body refers to the enclosing struct type.                               |

## Doc gaps observed

| Anchor                                                                                 | Kind            | Gap                                                                                                                                                                                      | Suggested addition                                                                                                                                |
| -------------------------------------------------------------------------------------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#this-expression](../../../../language-reference/expressions-this.md#this-expression) | missing-example | The doc describes `this` in instance methods, subscripts, and initializers but does not include a minimal example shader. The `> TODO` banner at the top confirms the doc is incomplete. | Add a short code snippet showing `this.field` access in a member function and in an `__init` constructor to ground the identity claim concretely. |
