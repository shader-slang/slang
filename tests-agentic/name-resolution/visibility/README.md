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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A `public` field of a less-visible (internal) struct type inside a `public` aggregate is rejected with `use-of-less-visible-type`. | negative | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap) | [`public-field-of-less-visible-type-rejected.slang`](public-field-of-less-visible-type-rejected.slang) |
| A `public` function whose return type is less-visible (an `internal` struct) is rejected with `use-of-less-visible-type`. | negative | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap) | [`public-function-with-internal-return-rejected.slang`](public-function-with-internal-return-rejected.slang) |
| A synthesized `$init` for a `public` struct that contains an `internal`-typed field is rejected with `references less visible type`. | negative | [#container-level-cap](../../../docs/llm-generated/name-resolution/visibility.md#container-level-cap) | [`synthesized-init-references-less-visible-type-rejected.slang`](synthesized-init-references-less-visible-type-rejected.slang) |
| A `public` member inside an `internal` aggregate is rejected by `checkVisibility` with `decl-cannot-have-higher-visibility`. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes) | [`public-inside-internal-struct-rejected.slang`](public-inside-internal-struct-rejected.slang) |
| `private` on a top-level decl (no enclosing aggregate or namespace) is rejected with `invalid-use-of-private-visibility`. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes) | [`private-on-top-level-decl-rejected.slang`](private-on-top-level-decl-rejected.slang) |
| `private` on an interface requirement is rejected; the doc's "private inside the declaring container" rule does not extend to interface requirements which must be visible to implementers. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes) | [`private-interface-requirement-rejected.slang`](private-interface-requirement-rejected.slang) |
| A `GenericDecl` takes its visibility from the inner decl; a public generic function is reachable through the public-on-inner classification. | functional | [#generic-parameters-accessors-and-synthesized-members](../../../docs/llm-generated/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members) | [`generic-function-visibility-from-inner.slang`](generic-function-visibility-from-inner.slang) |
| An `AccessorDecl` inherits visibility from its enclosing subscript/property; a private subscript's accessors are private and rejected from outside the struct. | negative | [#generic-parameters-accessors-and-synthesized-members](../../../docs/llm-generated/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members) | [`accessor-inherits-property-visibility.slang`](accessor-inherits-property-visibility.slang) |
| A `private` field is not visible outside its enclosing aggregate; access from a free function is rejected. | negative | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | [`private-field-rejected-from-outside-struct.slang`](private-field-rejected-from-outside-struct.slang) |
| A `private` member is visible from another method of the same aggregate; access through `this` succeeds. | functional | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | [`private-member-callable-from-same-struct.slang`](private-member-callable-from-same-struct.slang) |
| A `private` method is not visible outside its enclosing aggregate; an overload call from a free function is rejected. | negative | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | [`private-method-rejected-from-outside-struct.slang`](private-method-rejected-from-outside-struct.slang) |
| A `public` decl is always visible; `isDeclVisibleFromScope` short-circuits on Public and returns true. | functional | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | [`public-decl-callable-within-same-module.slang`](public-decl-callable-within-same-module.slang) |
| An `internal` decl is visible from any file in the same module; a free function calls another module-local `internal` function successfully. | functional | [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | [`internal-decl-callable-within-same-module.slang`](internal-decl-callable-within-same-module.slang) |
| A `private` method of struct A is not visible from a method of an unrelated struct B; `isDeclVisibleFromScope` walks B's parents and never finds A. | negative | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered) | [`private-method-rejected-from-other-struct.slang`](private-method-rejected-from-other-struct.slang) |
| A private member declared in one `extension S` is reachable from another `extension S` because both extensions resolve to the same target type. | functional | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered) | [`private-callable-from-sibling-extension.slang`](private-callable-from-sibling-extension.slang) |
| `isDeclVisibleFromScope` treats an extension on the same type as inside the aggregate; a private member is reachable from that extension. | functional | [#where-visibility-is-filtered](../../../docs/llm-generated/name-resolution/visibility.md#where-visibility-is-filtered) | [`private-callable-from-extension-on-same-type.slang`](private-callable-from-extension-on-same-type.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#per-keyword-semantics](../../../docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics) | undocumented-behavior | The "Per-keyword semantics" section claims `private` is visible "inside the declaring container — that is, the same aggregate type (`struct`, `class`, `interface`, ...) **or the same namespace**." In practice the compiler rejects a `private` decl declared directly inside a `namespace` block with `invalid-use-of-private-visibility` (because `isGlobalDecl(decl)` returns true for namespace-scope decls in `slang-check-modifier.cpp` around line 1971). | The doc should clarify that `private` works for aggregate-type members only, or describe the namespace-scope path that triggers the diagnostic. |
| [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes) | undocumented-behavior | The "Edge cases and failure modes" section lists `invalid-use-of-private-visibility` as firing for "a top-level decl … marked `private`," but the same diagnostic also fires for interface requirements (see `private-interface-requirement-rejected.slang`). A second bullet enumerating the interface-requirement case would make the failure-mode list complete. |  |
| [#defaults-by-language-version](../../../docs/llm-generated/name-resolution/visibility.md#defaults-by-language-version) | undocumented-behavior | "Defaults by language version" describes the legacy-language default of `public` for unannotated decls, but does not surface a single-file way to observe that default. Today the default is only observable across `import` boundaries between modules of different language versions, which the single-file agentic runner cannot express. |  |

## Out of scope

| Anchor | Reason | Claim | Why it's terminal |
| --- | --- | --- | --- |
| [#filterlookupresultbyvisibilityanddiagnose](../../../docs/llm-generated/name-resolution/visibility.md#filterlookupresultbyvisibilityanddiagnose) | (unclassified) | Language-server-mode "return unfiltered result" behaviour from `filterLookupResultByVisibilityAndDiagnose`. The agentic runner exercises `slangc` / `slangi`, not the LSP code path. | Not reachable via any allowed test directive. |
| [#import](../../../docs/llm-generated/name-resolution/visibility.md#import) | (unclassified) | Cross-module `import`-time visibility: confirming that an `internal` decl in module A is invisible from a separate module B's importing scope (the `getModuleDecl(decl) == getModuleDecl(scope)` branch of `isDeclVisibleFromScope`). This requires two separately-compiled translation units that the single-file agentic tests cannot host. Coverage exists in `tests/diagnostics/extension-visibility*.slang`. | Not reachable via any allowed test directive. |
| [#slanglanguageversionlegacy](../../../docs/llm-generated/name-resolution/visibility.md#slanglanguageversionlegacy) | (unclassified) | Legacy-language-default (`SLANG_LANGUAGE_VERSION_LEGACY`) `public` behaviour. Observing the default's effect requires a modern-language consumer importing a legacy module; same multi-file limitation. | Not reachable via any allowed test directive. |
