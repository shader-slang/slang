---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:30:00+00:00
source_commit: 74db89b9f77cdced9c4d0c47f377b38fffb9180b
watched_paths_digest: 47917e9e7bbd0d4c7cec93d02043b25787343696017d2490659c825a87e68dbe
source_doc: docs/llm-generated/pipeline/03-semantic-check.md
source_doc_digest: 7cb7a8661d1aff4316b63053539fe2827ef6a707dcb4338674753b06c8afbfa9
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/03-semantic-check

## Intent

Tests verify the semantic-checking behaviors described in
[`docs/llm-generated/pipeline/03-semantic-check.md`](../../../docs/llm-generated/pipeline/03-semantic-check.md):
turning a raw AST into a fully resolved, type-checked AST. The doc
hands off algorithmic detail for name resolution and overload
resolution to the
[`name-resolution/`](../../../docs/llm-generated/name-resolution/)
subtree, so a small number of tests anchor to those secondary docs
where the primary doc explicitly references them.

Semantic checking is dominantly about rejection. The bundle is
weighted toward `DIAGNOSTIC_TEST` (negative) coverage of the doc's
"these things are checked" claims: type mismatches, missing
interface members, control-flow rule violations, lvalue checks,
modifier-based mutability, generic-without-arguments, redeclaration,
ambiguous-reference, function-redefinition, divide-by-zero,
case-outside-switch, return-needs-expression, missing-return,
entry-point stage requirement. Functional `//TEST:INTERPRET` tests
cover the doc's positive claims: type-attached expressions, function
bodies parsed/checked on demand, container-level overload
accumulation, conversion-cost-based overload selection, generic
argument inference, interface conformance, implicit conversion,
typedef resolution, operator overloading, base-member lookup, and
synthesized initializer-list construction.

Multi-backend rule: every claim in this doc is target-independent
(the doc never asserts target-conditional semantic behavior), so
each test uses a single directive. Positive observations use
`//TEST:INTERPRET` (the lightest-weight runner that exercises
checked code at runtime); diagnostic claims use
`//DIAGNOSTIC_TEST:SIMPLE`.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                       | Claim (one line)                                                                                                          | Tests                                              |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------- |
| C-01     | [#inputs-and-outputs](../../../docs/llm-generated/pipeline/03-semantic-check.md#inputs-and-outputs)                                                          | After checking, every `Expr` carries a `Type*`; an int-float arithmetic site coerces via the resolved types.              | [`expr-gets-type-attached.slang`](expr-gets-type-attached.slang)                    |
| C-02     | [#inputs-and-outputs](../../../docs/llm-generated/pipeline/03-semantic-check.md#inputs-and-outputs)                                                          | Every `DeclRef`-bearing node points at the canonical decl after checking.                                                 | [`declref-points-at-canonical-decl.slang`](declref-points-at-canonical-decl.slang)           |
| C-03     | [#two-pass-interaction-with-the-parser](../../../docs/llm-generated/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser)                      | Function bodies parsed-on-demand are fully checked and run.                                                               | [`function-body-checked-after-parser-handoff.slang`](function-body-checked-after-parser-handoff.slang) |
| C-04     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Implicit-conversion ranking inserts an int→float coercion at a typed initialization site.                                 | [`implicit-int-to-float-conversion.slang`](implicit-int-to-float-conversion.slang)           |
| C-05     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Type-resolution: a typedef'd name resolves to its base type.                                                              | [`typedef-resolves-to-base-type.slang`](typedef-resolves-to-base-type.slang)              |
| C-06     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Statement checking: `break` outside a loop or switch is rejected (E30003).                                                | [`break-outside-loop-rejected.slang`](break-outside-loop-rejected.slang)                |
| C-07     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Statement checking: `continue` outside a loop is rejected (E30004).                                                       | [`continue-outside-loop-rejected.slang`](continue-outside-loop-rejected.slang)             |
| C-08     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Statement checking: `case` outside a `switch` is rejected.                                                                | [`case-outside-switch-rejected.slang`](case-outside-switch-rejected.slang)               |
| C-09     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Statement checking: a bare `return` in a non-void function is rejected (return-needs-expression).                         | [`return-needs-expression-rejected.slang`](return-needs-expression-rejected.slang)           |
| C-10     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Statement checking: a non-void function that falls off the end triggers the missing-return warning (W41010).              | [`missing-return-warning.slang`](missing-return-warning.slang)                     |
| C-11     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Expression checking: assigning to a non-lvalue expression is rejected (E30011).                                           | [`assign-to-non-lvalue-rejected.slang`](assign-to-non-lvalue-rejected.slang)              |
| C-12     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Expression checking: calling a non-callable value is rejected (E30016).                                                   | [`call-on-non-callable-rejected.slang`](call-on-non-callable-rejected.slang)              |
| C-13     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Expression checking: a constant divide-by-zero is rejected (E30002).                                                      | [`divide-by-zero-rejected.slang`](divide-by-zero-rejected.slang)                    |
| C-14     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Type checking: `void` is not a valid value type (E30009).                                                                 | [`invalid-type-void-rejected.slang`](invalid-type-void-rejected.slang)                 |
| C-15     | [#files-and-responsibilities](../../../docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities)                                          | Decl checking: two locals with the same name in one block are reported as conflicting declarations (E30200).              | [`local-redeclaration-rejected.slang`](local-redeclaration-rejected.slang)               |
| C-16     | [#modifier-validation](../../../docs/llm-generated/pipeline/03-semantic-check.md#modifier-validation)                                                        | `let`-bound locals are immutable; assignment to one is rejected as non-lvalue.                                            | [`assign-to-let-is-rejected.slang`](assign-to-let-is-rejected.slang)                  |
| C-17     | [#modifier-validation](../../../docs/llm-generated/pipeline/03-semantic-check.md#modifier-validation)                                                        | A `[mutating]` method cannot be called on a const (immutable) receiver (E30050).                                          | [`mutating-method-on-immutable-rejected.slang`](mutating-method-on-immutable-rejected.slang)      |
| C-18     | [#generic-specialization-and-constraints](../../../docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints)                  | A generic type used without arguments is rejected (generic-type-needs-args).                                              | [`generic-without-args-rejected.slang`](generic-without-args-rejected.slang)              |
| C-19     | [#generic-specialization-and-constraints](../../../docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints)                  | Constraint solver infers the type argument of a generic from the call's actual argument.                                  | [`generic-argument-inferred-from-call.slang`](generic-argument-inferred-from-call.slang)        |
| C-20     | [#generic-specialization-and-constraints](../../../docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints)                  | Conformance checker accepts a struct that implements an interface as a witness for a constrained generic.                 | [`interface-conformance-checked.slang`](interface-conformance-checked.slang)              |
| C-21     | [#generic-specialization-and-constraints](../../../docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints)                  | Conformance checker rejects a struct declared as conforming but missing required members (E38100).                        | [`missing-conformance-rejected.slang`](missing-conformance-rejected.slang)               |
| C-22     | [#synthesizing-implicit-code](../../../docs/llm-generated/pipeline/03-semantic-check.md#synthesizing-implicit-code)                                          | The checker synthesizes initializer-list construction so `Point p = {3, 4};` works without an explicit `__init`.          | [`synthesized-positional-constructor.slang`](synthesized-positional-constructor.slang)         |
| C-23     | [#shader-specific-checks](../../../docs/llm-generated/pipeline/03-semantic-check.md#shader-specific-checks)                                                  | Entry-point checking requires a stage; an entry point with no `[shader(...)]` attribute and no `-stage` arg is rejected (E38007). | [`entry-point-needs-stage-rejected.slang`](entry-point-needs-stage-rejected.slang)           |
| C-24     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | Unresolved names flow through `DiagnosticSink` as undefined-identifier (E30015); the checker replaces them with ErrorType. | [`undefined-identifier-becomes-error.slang`](undefined-identifier-becomes-error.slang)         |
| C-25     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | A type mismatch at a coercion site is reported with both expected and actual types (E30019).                              | [`type-mismatch-rejected.slang`](type-mismatch-rejected.slang)                     |
| C-26     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | Member lookup that finds no match emits "member not found" (E30027) naming the missing member and the host type.          | [`member-lookup-no-match.slang`](member-lookup-no-match.slang)                     |
| C-27     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | A redefined function body is rejected with the function-redefinition diagnostic + a "see previous definition" note (E30201). | [`function-redefinition-diagnostic.slang`](function-redefinition-diagnostic.slang)           |
| C-28     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | A struct attempting to inherit from a sealed builtin (e.g. `int`) is rejected.                                            | [`inheritance-from-non-interface-rejected.slang`](inheritance-from-non-interface-rejected.slang)    |
| C-29     | [#failure-modes](../../../docs/llm-generated/pipeline/03-semantic-check.md#failure-modes)                                                                    | Check recovery does not cascade: a later, unrelated error still surfaces after an earlier one was reported.               | [`recovery-after-error-does-not-cascade.slang`](recovery-after-error-does-not-cascade.slang)      |
| C-30     | [#container-level-overload-accumulation](../../../docs/llm-generated/name-resolution/lookup.md#container-level-overload-accumulation)                        | Same-name functions in a container accumulate into an overload set; the checker picks each by argument type.              | [`overload-accumulation-by-arg-type.slang`](overload-accumulation-by-arg-type.slang)          |
| C-31     | [#conversion-costs](../../../docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs)                                                     | Overload resolution prefers the candidate with the lowest total conversion cost.                                          | [`overload-prefer-exact-type.slang`](overload-prefer-exact-type.slang)                 |
| C-32     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes)                             | A call with no applicable candidate is rejected with a "not enough arguments" diagnostic.                                 | [`no-applicable-overload-diagnostic.slang`](no-applicable-overload-diagnostic.slang)          |
| C-33     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes)                                          | A non-overloadable lookup resolving to multiple decls is reported as ambiguous-reference.                                 | [`ambiguous-reference-diagnostic.slang`](ambiguous-reference-diagnostic.slang)             |
| C-34     | [#block-local-shadowing](../../../docs/llm-generated/name-resolution/lookup.md#block-local-shadowing)                                                        | Block-local shadowing hides a local until its DeclStmt; using `x` before its declaration produces undefined-identifier.   | [`forward-reference-in-block-rejected.slang`](forward-reference-in-block-rejected.slang)        |
| C-35     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                        | Member lookup walks inheritance facets so a base-interface method is callable on a conforming struct value.               | [`inheritance-walk-finds-base-member.slang`](inheritance-walk-finds-base-member.slang)         |
| C-36     | [#operator-overloading](../../../docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading)                                             | Operator overloading: a user-defined `operator+` on a struct is selected by overload resolution at a `+` expression.       | [`operator-overload-on-struct.slang`](operator-overload-on-struct.slang)                |

## Tests in this bundle

| File                                               | Intent     | Doc anchor                                  |
| -------------------------------------------------- | ---------- | ------------------------------------------- |
| [`ambiguous-reference-diagnostic.slang`](ambiguous-reference-diagnostic.slang)             | negative   | `#edge-cases-and-failure-modes`             |
| [`assign-to-let-is-rejected.slang`](assign-to-let-is-rejected.slang)                  | negative   | `#modifier-validation`                      |
| [`assign-to-non-lvalue-rejected.slang`](assign-to-non-lvalue-rejected.slang)              | negative   | `#files-and-responsibilities`               |
| [`break-outside-loop-rejected.slang`](break-outside-loop-rejected.slang)                | negative   | `#files-and-responsibilities`               |
| [`call-on-non-callable-rejected.slang`](call-on-non-callable-rejected.slang)              | negative   | `#files-and-responsibilities`               |
| [`case-outside-switch-rejected.slang`](case-outside-switch-rejected.slang)               | negative   | `#files-and-responsibilities`               |
| [`continue-outside-loop-rejected.slang`](continue-outside-loop-rejected.slang)             | negative   | `#files-and-responsibilities`               |
| [`declref-points-at-canonical-decl.slang`](declref-points-at-canonical-decl.slang)           | functional | `#inputs-and-outputs`                       |
| [`divide-by-zero-rejected.slang`](divide-by-zero-rejected.slang)                    | negative   | `#files-and-responsibilities`               |
| [`entry-point-needs-stage-rejected.slang`](entry-point-needs-stage-rejected.slang)           | negative   | `#shader-specific-checks`                   |
| [`expr-gets-type-attached.slang`](expr-gets-type-attached.slang)                    | functional | `#inputs-and-outputs`                       |
| [`forward-reference-in-block-rejected.slang`](forward-reference-in-block-rejected.slang)        | negative   | `#block-local-shadowing`                    |
| [`function-body-checked-after-parser-handoff.slang`](function-body-checked-after-parser-handoff.slang) | functional | `#two-pass-interaction-with-the-parser`     |
| [`function-redefinition-diagnostic.slang`](function-redefinition-diagnostic.slang)           | negative   | `#failure-modes`                            |
| [`generic-argument-inferred-from-call.slang`](generic-argument-inferred-from-call.slang)        | functional | `#generic-specialization-and-constraints`   |
| [`generic-without-args-rejected.slang`](generic-without-args-rejected.slang)              | negative   | `#generic-specialization-and-constraints`   |
| [`implicit-int-to-float-conversion.slang`](implicit-int-to-float-conversion.slang)           | functional | `#files-and-responsibilities`               |
| [`inheritance-from-non-interface-rejected.slang`](inheritance-from-non-interface-rejected.slang)    | negative   | `#failure-modes`                            |
| [`inheritance-walk-finds-base-member.slang`](inheritance-walk-finds-base-member.slang)         | functional | `#member-lookup`                            |
| [`interface-conformance-checked.slang`](interface-conformance-checked.slang)              | functional | `#generic-specialization-and-constraints`   |
| [`invalid-type-void-rejected.slang`](invalid-type-void-rejected.slang)                 | negative   | `#files-and-responsibilities`               |
| [`local-redeclaration-rejected.slang`](local-redeclaration-rejected.slang)               | negative   | `#files-and-responsibilities`               |
| [`member-lookup-no-match.slang`](member-lookup-no-match.slang)                     | negative   | `#failure-modes`                            |
| [`missing-conformance-rejected.slang`](missing-conformance-rejected.slang)               | negative   | `#generic-specialization-and-constraints`   |
| [`missing-return-warning.slang`](missing-return-warning.slang)                     | negative   | `#files-and-responsibilities`               |
| [`mutating-method-on-immutable-rejected.slang`](mutating-method-on-immutable-rejected.slang)      | negative   | `#modifier-validation`                      |
| [`no-applicable-overload-diagnostic.slang`](no-applicable-overload-diagnostic.slang)          | negative   | `#edge-cases-and-failure-modes`             |
| [`operator-overload-on-struct.slang`](operator-overload-on-struct.slang)                | functional | `#operator-overloading`                     |
| [`overload-accumulation-by-arg-type.slang`](overload-accumulation-by-arg-type.slang)          | functional | `#container-level-overload-accumulation`    |
| [`overload-prefer-exact-type.slang`](overload-prefer-exact-type.slang)                 | functional | `#conversion-costs`                         |
| [`recovery-after-error-does-not-cascade.slang`](recovery-after-error-does-not-cascade.slang)      | negative   | `#failure-modes`                            |
| [`return-needs-expression-rejected.slang`](return-needs-expression-rejected.slang)           | negative   | `#files-and-responsibilities`               |
| [`synthesized-positional-constructor.slang`](synthesized-positional-constructor.slang)         | functional | `#synthesizing-implicit-code`               |
| [`type-mismatch-rejected.slang`](type-mismatch-rejected.slang)                     | negative   | `#failure-modes`                            |
| [`typedef-resolves-to-base-type.slang`](typedef-resolves-to-base-type.slang)              | functional | `#files-and-responsibilities`               |
| [`undefined-identifier-becomes-error.slang`](undefined-identifier-becomes-error.slang)         | negative   | `#failure-modes`                            |
| [`ambiguous-overload-rejected.slang`](ambiguous-overload-rejected.slang)                | negative   | `#edge-cases-and-failure-modes`             |
| [`argument-must-be-lvalue-rejected.slang`](argument-must-be-lvalue-rejected.slang)           | negative   | `#files-and-responsibilities`               |
| [`array-negative-size-rejected.slang`](array-negative-size-rejected.slang)               | boundary   | `#files-and-responsibilities`               |
| [`attribute-not-applicable-rejected.slang`](attribute-not-applicable-rejected.slang)          | negative   | `#modifier-validation`                      |
| [`bit-field-non-integral-rejected.slang`](bit-field-non-integral-rejected.slang)            | negative   | `#files-and-responsibilities`               |
| [`bit-field-too-wide-rejected.slang`](bit-field-too-wide-rejected.slang)                | boundary   | `#files-and-responsibilities`               |
| [`break-inside-defer-rejected.slang`](break-inside-defer-rejected.slang)                | negative   | `#files-and-responsibilities`               |
| [`class-without-new-rejected.slang`](class-without-new-rejected.slang)                 | negative   | `#files-and-responsibilities`               |
| [`conflicting-struct-redeclaration-rejected.slang`](conflicting-struct-redeclaration-rejected.slang)  | negative   | `#files-and-responsibilities`               |
| [`continue-inside-defer-rejected.slang`](continue-inside-defer-rejected.slang)             | negative   | `#files-and-responsibilities`               |
| [`cyclic-inheritance-rejected.slang`](cyclic-inheritance-rejected.slang)                | negative   | `#failure-modes`                            |
| [`default-argument-with-zero-args.slang`](default-argument-with-zero-args.slang)            | boundary   | `#conversion-costs`                         |
| [`duplicate-modifier-rejected.slang`](duplicate-modifier-rejected.slang)                | negative   | `#modifier-validation`                      |
| [`expected-string-literal-attr-rejected.slang`](expected-string-literal-attr-rejected.slang)      | negative   | `#shader-specific-checks`                   |
| [`function-return-type-mismatch-rejected.slang`](function-return-type-mismatch-rejected.slang)     | negative   | `#failure-modes`                            |
| [`generic-value-parameter-type-rejected.slang`](generic-value-parameter-type-rejected.slang)      | negative   | `#generic-specialization-and-constraints`   |
| [`global-var-opaque-type-rejected.slang`](global-var-opaque-type-rejected.slang)            | negative   | `#modifier-validation`                      |
| [`invalid-extension-on-interface-rejected.slang`](invalid-extension-on-interface-rejected.slang)    | negative   | `#generic-specialization-and-constraints`   |
| [`invalid-type-for-constraint-rejected.slang`](invalid-type-for-constraint-rejected.slang)       | negative   | `#generic-specialization-and-constraints`   |
| [`many-args-positional.slang`](many-args-positional.slang)                       | boundary   | `#conversion-costs`                         |
| [`member-not-found-on-struct-rejected.slang`](member-not-found-on-struct-rejected.slang)        | negative   | `#failure-modes`                            |
| [`missing-override-rejected.slang`](missing-override-rejected.slang)                  | negative   | `#generic-specialization-and-constraints`   |
| [`new-on-non-class-rejected.slang`](new-on-non-class-rejected.slang)                  | negative   | `#files-and-responsibilities`               |
| [`no-applicable-overload-zero-candidate-rejected.slang`](no-applicable-overload-zero-candidate-rejected.slang) | negative | `#edge-cases-and-failure-modes`            |
| [`numthreads-non-positive-rejected.slang`](numthreads-non-positive-rejected.slang)           | boundary   | `#shader-specific-checks`                   |
| [`overload-prefer-int-over-uint.slang`](overload-prefer-int-over-uint.slang)              | boundary   | `#conversion-costs`                         |
| [`pack-param-must-be-last-rejected.slang`](pack-param-must-be-last-rejected.slang)           | negative   | `#generic-specialization-and-constraints`   |
| [`return-inside-defer-rejected.slang`](return-inside-defer-rejected.slang)               | negative   | `#files-and-responsibilities`               |
| [`spec-constant-must-be-scalar-rejected.slang`](spec-constant-must-be-scalar-rejected.slang)      | negative   | `#modifier-validation`                      |
| [`subscript-on-non-array-rejected.slang`](subscript-on-non-array-rejected.slang)            | negative   | `#files-and-responsibilities`               |
| [`switch-duplicate-cases-rejected.slang`](switch-duplicate-cases-rejected.slang)            | negative   | `#files-and-responsibilities`               |
| [`switch-multiple-default-rejected.slang`](switch-multiple-default-rejected.slang)           | negative   | `#files-and-responsibilities`               |
| [`synthesized-positional-constructor-nested.slang`](synthesized-positional-constructor-nested.slang)  | boundary   | `#synthesizing-implicit-code`               |
| [`too-many-arguments-rejected.slang`](too-many-arguments-rejected.slang)                | negative   | `#edge-cases-and-failure-modes`             |
| [`too-many-initializers-rejected.slang`](too-many-initializers-rejected.slang)             | boundary   | `#synthesizing-implicit-code`               |
| [`try-on-non-call-rejected.slang`](try-on-non-call-rejected.slang)                   | negative   | `#files-and-responsibilities`               |
| [`typedef-chain-resolves.slang`](typedef-chain-resolves.slang)                     | boundary   | `#files-and-responsibilities`               |
| [`uncaught-try-in-non-throw-rejected.slang`](uncaught-try-in-non-throw-rejected.slang)         | negative   | `#files-and-responsibilities`               |
| [`unsized-member-must-be-last-rejected.slang`](unsized-member-must-be-last-rejected.slang)       | negative   | `#files-and-responsibilities`               |
| [`variable-used-in-own-definition-rejected.slang`](variable-used-in-own-definition-rejected.slang)   | negative   | `#failure-modes`                            |

## Doc gaps observed

- The `## Synthesizing implicit code` section names "generated
  comparison / construction methods" as a category but does not
  enumerate which comparison operators are auto-synthesized
  (`==`, `!=`, `<`, `>`, `<=`, `>=`) nor which structural rules
  trigger them. The bundle tests the construction half (initializer-
  list constructor) but not comparison-method synthesis; a doc
  enumeration would let the bundle grow exhaustively in that area.
- The `## Shader-specific checks` section lists "parameter modifiers
  (`in`, `out`, `inout` and stage-specific intrinsics)" and "return
  type compatibility with the stage" as checked, but the doc does
  not promise specific diagnostic codes or message text. The
  `entry-point-needs-stage` test anchors to the one concrete claim
  (a stage attribute or `-stage` is required); the broader
  parameter-modifier / return-type-per-stage claims need a doc
  enumeration before a test can anchor to them.
- The `## Modifier validation` section says
  slang-check-modifier.cpp validates "which modifiers are allowed on
  which decls, mutually exclusive combinations, attribute argument
  types, and HLSL-vs-Slang dialect differences". No specific
  mutually-exclusive pair is named in the doc. Tests for individual
  combos (e.g. `static [mutating]`) would be anchoring to undocumented
  behavior; deferred until the doc lists at least one explicit
  exclusion.
- The `## Failure modes` section says overload resolution "returns a
  synthetic `errorExpr` rather than aborting". The user-visible
  consequence is "compilation continues past the offending site" —
  the `recovery-after-error-does-not-cascade` test exercises the
  general claim, but the doc has no observable surface that
  specifically distinguishes "synthetic errorExpr" from any other
  error-recovery placeholder.
- The `## SemanticsVisitor` and `## Files and responsibilities`
  sections describe the architectural split (which `.cpp` file
  handles which concern). Most of those claims are not directly
  observable from outside the compiler — the bundle anchors negative
  tests to the file table by treating each row's "concern" as a
  testable surface claim (e.g. "statement checking validates
  control-flow rules"). A stronger anchoring would be to add a
  per-concern subsection enumerating exactly which user-visible
  behaviors live in each file.
- The doc points readers to
  [`../name-resolution/`](../../../docs/llm-generated/name-resolution/)
  for the lookup algorithm but does not restate any specific
  consequences in this file. The bundle therefore anchors a handful
  of tests directly to `lookup.md` and `overload-resolution.md`
  (which the per-section prompt explicitly permits). If the source
  doc were extended with one-line summaries of each user-visible
  lookup behavior, those tests could re-anchor here.

## Out of scope (no-GPU runner)

None for this bundle. Semantic-checking behaviors are target-
independent (the checker runs before any backend-specific lowering),
so `slangi` and `slangc` with text targets observe every documented
claim without a GPU.
