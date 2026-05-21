---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T18:00:00+00:00
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 05ad86d18030b1168bee813183c25bea8c475d13eaac6270fbcbeb5bba674021
source_doc: docs/llm-generated/name-resolution/visibility.md
source_doc_digest: 70745a6878e4f9b04fafb95dcc2d083c2dd3f9a31162f286e62accbb336ed014
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for name-resolution/visibility

## Intent

Tests verify the visibility-enforcement behaviors described in
[`docs/llm-generated/name-resolution/visibility.md`](../../../docs/llm-generated/name-resolution/visibility.md):
which decl/scope pairs are accepted by
`SemanticsVisitor::isDeclVisibleFromScope`, which pairs are rejected
with `declaration not accessible` at the lookup boundary or in
overload resolution, and which declarations are rejected outright by
`SemanticsVisitor::checkVisibility` (higher-than-parent,
less-visible-type, invalid-private).

The bundle pairs each enforcement claim with a **positive** test
that proves access is permitted from inside the documented container
and a **negative** test that proves the same access is rejected from
outside. Positive cases use `//TEST:INTERPRET(filecheck=CHECK):`
because visibility is fixed at semantic check before any backend-
specific lowering. Negative cases use
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` to pin the rejection text
verbatim (with caret columns matching the runner's "Suggested
annotations" output). Multi-backend testing is not used: visibility
filtering is target-independent.

This bundle is deliberately **single-file**: every test is a single
`.slang` source. Cross-module `import`-time visibility (an `internal`
decl invisible from a different module) cannot be expressed in a
single file with the agentic runner and is captured under
"Out of scope (single-file runner)" below.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                | Claim (one line)                                                                                                  | Tests                                                                  |
| -------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------- |
| C-01     | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics)                                                             | A `public` decl is universally visible; same-module access succeeds.                                              | [`public-decl-callable-within-same-module.slang`](public-decl-callable-within-same-module.slang)                        |
| C-02     | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics)                                                             | An `internal` decl is visible from another decl in the same module.                                               | [`internal-decl-callable-within-same-module.slang`](internal-decl-callable-within-same-module.slang)                      |
| C-03     | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics)                                                             | A `private` field of a struct is visible to a sibling method of the same struct.                                  | [`private-member-callable-from-same-struct.slang`](private-member-callable-from-same-struct.slang)                       |
| C-04     | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics)                                                             | A `private` field is not visible from a free function outside the struct.                                         | [`private-field-rejected-from-outside-struct.slang`](private-field-rejected-from-outside-struct.slang)                     |
| C-05     | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics)                                                             | A `private` method is not visible from a free function; overload resolution rejects it.                           | [`private-method-rejected-from-outside-struct.slang`](private-method-rejected-from-outside-struct.slang)                    |
| C-06     | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered)                                               | A `private` method of struct A is not visible from a method of an unrelated struct B.                             | [`private-method-rejected-from-other-struct.slang`](private-method-rejected-from-other-struct.slang)                      |
| C-07     | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered)                                               | A `private` struct member is reachable from a method declared in `extension S` on the same type.                  | [`private-callable-from-extension-on-same-type.slang`](private-callable-from-extension-on-same-type.slang)                   |
| C-08     | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered)                                               | A `private` member declared in one `extension S` is reachable from a sibling `extension S`.                       | [`private-callable-from-sibling-extension.slang`](private-callable-from-sibling-extension.slang)                        |
| C-09     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes)                                               | A `public` member inside an `internal`-default aggregate is rejected with `decl-cannot-have-higher-visibility`.   | [`public-inside-internal-struct-rejected.slang`](public-inside-internal-struct-rejected.slang)                         |
| C-10     | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap)                                                                 | A `public` function whose return type is `internal` is rejected with `use-of-less-visible-type`.                  | [`public-function-with-internal-return-rejected.slang`](public-function-with-internal-return-rejected.slang)                  |
| C-11     | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap)                                                                 | A `public` field of `internal` type inside a `public` aggregate is rejected with `use-of-less-visible-type`.      | [`public-field-of-less-visible-type-rejected.slang`](public-field-of-less-visible-type-rejected.slang)                     |
| C-12     | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap)                                                                 | The synthesized `$init` of a `public` aggregate with an `internal`-typed field is rejected.                       | [`synthesized-init-references-less-visible-type-rejected.slang`](synthesized-init-references-less-visible-type-rejected.slang)         |
| C-13     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes)                                               | `private` on a top-level decl (no enclosing aggregate) is rejected with `invalid-use-of-private-visibility`.      | [`private-on-top-level-decl-rejected.slang`](private-on-top-level-decl-rejected.slang)                             |
| C-14     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes)                                               | `private` on an interface requirement is rejected with `invalid-use-of-private-visibility`.                       | [`private-interface-requirement-rejected.slang`](private-interface-requirement-rejected.slang)                         |
| C-15     | [#generic-parameters-accessors-and-synthesized-members](../../../docs/llm-generated/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members) | An `AccessorDecl` inherits visibility from its enclosing subscript; a `private __subscript` is rejected.        | [`accessor-inherits-property-visibility.slang`](accessor-inherits-property-visibility.slang)                          |
| C-16     | [#generic-parameters-accessors-and-synthesized-members](../../../docs/llm-generated/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members) | A `GenericDecl`'s visibility is that of its inner decl; a `public` generic function is callable.                | [`generic-function-visibility-from-inner.slang`](generic-function-visibility-from-inner.slang)                         |

## Tests in this bundle

| File                                                              | Intent     | Doc anchor                                              |
| ----------------------------------------------------------------- | ---------- | ------------------------------------------------------- |
| [`accessor-inherits-property-visibility.slang`](accessor-inherits-property-visibility.slang)                     | negative   | `#generic-parameters-accessors-and-synthesized-members` |
| [`generic-function-visibility-from-inner.slang`](generic-function-visibility-from-inner.slang)                    | functional | `#generic-parameters-accessors-and-synthesized-members` |
| [`internal-decl-callable-within-same-module.slang`](internal-decl-callable-within-same-module.slang)                 | functional | `#per-keyword-semantics`                                |
| [`private-callable-from-extension-on-same-type.slang`](private-callable-from-extension-on-same-type.slang)              | functional | `#where-visibility-is-filtered`                         |
| [`private-callable-from-sibling-extension.slang`](private-callable-from-sibling-extension.slang)                   | functional | `#where-visibility-is-filtered`                         |
| [`private-field-rejected-from-outside-struct.slang`](private-field-rejected-from-outside-struct.slang)                | negative   | `#per-keyword-semantics`                                |
| [`private-interface-requirement-rejected.slang`](private-interface-requirement-rejected.slang)                    | negative   | `#edge-cases-and-failure-modes`                         |
| [`private-member-callable-from-same-struct.slang`](private-member-callable-from-same-struct.slang)                  | functional | `#per-keyword-semantics`                                |
| [`private-method-rejected-from-other-struct.slang`](private-method-rejected-from-other-struct.slang)                 | negative   | `#where-visibility-is-filtered`                         |
| [`private-method-rejected-from-outside-struct.slang`](private-method-rejected-from-outside-struct.slang)               | negative   | `#per-keyword-semantics`                                |
| [`private-on-top-level-decl-rejected.slang`](private-on-top-level-decl-rejected.slang)                        | negative   | `#edge-cases-and-failure-modes`                         |
| [`public-decl-callable-within-same-module.slang`](public-decl-callable-within-same-module.slang)                   | functional | `#per-keyword-semantics`                                |
| [`public-field-of-less-visible-type-rejected.slang`](public-field-of-less-visible-type-rejected.slang)                | negative   | `#container-level-cap`                                  |
| [`public-function-with-internal-return-rejected.slang`](public-function-with-internal-return-rejected.slang)             | negative   | `#container-level-cap`                                  |
| [`public-inside-internal-struct-rejected.slang`](public-inside-internal-struct-rejected.slang)                    | negative   | `#edge-cases-and-failure-modes`                         |
| [`synthesized-init-references-less-visible-type-rejected.slang`](synthesized-init-references-less-visible-type-rejected.slang)    | negative   | `#container-level-cap`                                  |

## Doc gaps observed

- The "Per-keyword semantics" section claims `private` is visible
  "inside the declaring container — that is, the same aggregate
  type (`struct`, `class`, `interface`, ...) **or the same
  namespace**." In practice the compiler rejects a `private` decl
  declared directly inside a `namespace` block with
  `invalid-use-of-private-visibility` (because
  `isGlobalDecl(decl)` returns true for namespace-scope decls in
  `slang-check-modifier.cpp` around line 1971). The doc should
  clarify that `private` works for aggregate-type members only,
  or describe the namespace-scope path that triggers the
  diagnostic.
- The "Edge cases and failure modes" section lists
  `invalid-use-of-private-visibility` as firing for "a top-level
  decl … marked `private`," but the same diagnostic also fires
  for interface requirements (see
  `private-interface-requirement-rejected.slang`). A second
  bullet enumerating the interface-requirement case would make
  the failure-mode list complete.
- "Defaults by language version" describes the legacy-language
  default of `public` for unannotated decls, but does not surface
  a single-file way to observe that default. Today the default is
  only observable across `import` boundaries between modules of
  different language versions, which the single-file agentic
  runner cannot express.

## Out of scope (single-file runner)

- Cross-module `import`-time visibility: confirming that an
  `internal` decl in module A is invisible from a separate module
  B's importing scope (the `getModuleDecl(decl) == getModuleDecl(scope)`
  branch of `isDeclVisibleFromScope`). This requires two
  separately-compiled translation units that the single-file
  agentic tests cannot host. Coverage exists in
  `tests/diagnostics/extension-visibility*.slang`.
- Legacy-language-default (`SLANG_LANGUAGE_VERSION_LEGACY`)
  `public` behaviour. Observing the default's effect requires a
  modern-language consumer importing a legacy module; same
  multi-file limitation.
- Language-server-mode "return unfiltered result" behaviour from
  `filterLookupResultByVisibilityAndDiagnose`. The agentic runner
  exercises `slangc` / `slangi`, not the LSP code path.
