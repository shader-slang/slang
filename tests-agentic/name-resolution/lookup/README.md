---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T13:48:38Z
source_commit: 1e0d460c1cb410005c4f775ba11fbc803cc8c16d
watched_paths_digest: 8143fa2272e298238feaa8bd9cff5f3d9a9b312b8c4305f415b3c6577ae77cac
source_doc: docs/llm-generated/name-resolution/lookup.md
source_doc_digest: 6fd4cef36add7d8029dbb7935cd0f95c2b7ed982a931a183f7031439fbcba517
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for name-resolution/lookup

## Intent

Tests verify the lookup-algorithm behaviors documented in
[`docs/llm-generated/name-resolution/lookup.md`](../../../docs/llm-generated/name-resolution/lookup.md):
the per-step walk of `_lookUpInScopes`, the `LookupMask` category
filter, the inheritance walk for member lookup, transparent-member
injection, pointer-like auto-dereference, breadcrumb construction
(This / SuperType / Member / Deref), `ThisParameterMode` updates
for `[mutating]` / `static` / `__init`, container-level overload
accumulation, the merge of re-opened namespaces and
`using namespace`, override-wins-over-default for interface defaults,
and the documented failure modes (ambiguous reference, member-not-
found, ErrorType silence, IgnoreForLookupModifier on enum tag-type,
forward-reference inside a block).

The bundle pairs algorithm-shape claims with positive
`//TEST:INTERPRET` tests (a `printf` echo of the resolved value) and
failure-mode claims with `//DIAGNOSTIC_TEST:SIMPLE` tests (the
documented diagnostic at the use site). One transparent-member and
one pointer-like-auto-deref test use `//TEST:SIMPLE -target hlsl`
because `cbuffer`/`ConstantBuffer` cannot be hosted in `slangi`
(it asks for VM bytecode for unsupported globals).

Multi-backend rule: lookup runs at semantic-check, before any
backend lowering. Per `_common.md`, INTERPRET is the primary
single-directive form. Only the transparent / deref tests use a
text-emit target, and only because the surface (`cbuffer` /
`ConstantBuffer<T>`) is not interpretable.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                | Claim (one line)                                                                                                              | Tests                                                            |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| L-01     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 1: outer loop walks `request.scope -> parent` to the module root.                                                        | [`unqualified-walks-parent-chain.slang`](unqualified-walks-parent-chain.slang)                           |
| L-02     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 6: variable hits are non-overloadable so they short-circuit the outward walk.                                            | [`unqualified-short-circuits-on-variable.slang`](unqualified-short-circuits-on-variable.slang)                   |
| L-03     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 6: function hits are overloadable so they accumulate from outer scopes.                                                  | [`unqualified-functions-accumulate-across-scopes.slang`](unqualified-functions-accumulate-across-scopes.slang)           |
| L-04     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 4 AggType-rewrite still fires when the use site is several block scopes deep.                                            | [`agg-type-this-rewrite-from-nested-block.slang`](agg-type-this-rewrite-from-nested-block.slang)                  |
| L-05     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 5 updates ThisParameterMode for `[mutating]`; unqualified writes mutate the receiver.                                    | [`this-breadcrumb-mutating-mode.slang`](this-breadcrumb-mutating-mode.slang)                            |
| L-06     | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup)                                                       | Step 5 updates ThisParameterMode for `static`; a static member resolves to the enclosing type's static field.                 | [`this-breadcrumb-static-type-mode.slang`](this-breadcrumb-static-type-mode.slang)                         |
| L-07     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | lookUpMember on a DeclRefType resolves a direct field of the type.                                                            | [`member-lookup-resolves-direct-field.slang`](member-lookup-resolves-direct-field.slang)                      |
| L-08     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | The inheritance walk reaches a base struct's field through a derived value.                                                   | [`member-lookup-walks-struct-inheritance.slang`](member-lookup-walks-struct-inheritance.slang)                   |
| L-09     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | The inheritance walk reaches an interface-declared method through a conforming struct receiver.                               | [`member-lookup-walks-interface-inheritance.slang`](member-lookup-walks-interface-inheritance.slang)                |
| L-10     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | The inheritance walk traverses transitive super-interfaces (IA <- IB <- IC).                                                  | [`inheritance-walk-traverses-multi-level-chain.slang`](inheritance-walk-traverses-multi-level-chain.slang)             |
| L-11     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | Member lookup on a pointer-like ConstantBuffer<T> auto-dereferences via a Deref breadcrumb.                                   | [`pointer-like-auto-deref-constant-buffer.slang`](pointer-like-auto-deref-constant-buffer.slang)                  |
| L-12     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | Same-type extension methods accumulate with direct members into one overload set on member lookup.                            | [`extension-and-direct-member-merge-into-overload-set.slang`](extension-and-direct-member-merge-into-overload-set.slang)      |
| L-13     | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup)                                                                 | Member lookup that finds no candidate produces `member not found` (code 30027) naming the missing identifier and host type.   | [`member-not-found-diagnostic.slang`](member-not-found-diagnostic.slang)                              |
| L-14     | [#transparent-members](../../../docs/llm-generated/name-resolution/lookup.md#transparent-members)                                                     | A `cbuffer` lowers to `__transparent ConstantBuffer<...>`; unqualified field reference resolves via Deref + Member breadcrumb.| [`transparent-cbuffer-resolves-field.slang`](transparent-cbuffer-resolves-field.slang)                       |
| L-15     | [#breadcrumbs](../../../docs/llm-generated/name-resolution/lookup.md#breadcrumbs)                                                                     | An unqualified field reference inside a method records a This breadcrumb so the rewrite reads `this.field`.                   | [`breadcrumb-this-implicit-in-method.slang`](breadcrumb-this-implicit-in-method.slang)                       |
| L-16     | [#container-level-overload-accumulation](../../../docs/llm-generated/name-resolution/lookup.md#container-level-overload-accumulation)                 | Same-name methods inside one struct container accumulate via `_prevInContainerWithSameName`.                                  | [`container-overload-accumulates-struct-methods.slang`](container-overload-accumulates-struct-methods.slang)            |
| L-17     | [#module-and-namespace](../../../docs/llm-generated/name-resolution/lookup.md#module-and-namespace)                                                   | `UsingDecl` injects a namespace into the sibling chain so its members are reachable unqualified.                              | [`namespace-merge-via-using.slang`](namespace-merge-via-using.slang)                                |
| L-18     | [#module-and-namespace](../../../docs/llm-generated/name-resolution/lookup.md#module-and-namespace)                                                   | Re-opened namespace blocks collapse into one NamespaceDecl; a single `using namespace` injects all accumulated members.       | [`namespace-reopened-then-using-injects-all.slang`](namespace-reopened-then-using-injects-all.slang)                |
| L-19     | [#interface-requirements-vs-default-implementations](../../../docs/llm-generated/name-resolution/lookup.md#interface-requirements-vs-default-implementations) | A conforming-type override of an interface method shadows the interface default implementation.                              | [`interface-override-wins-over-default.slang`](interface-override-wins-over-default.slang)                     |
| L-20     | [#generic-parameters](../../../docs/llm-generated/name-resolution/lookup.md#generic-parameters)                                                       | A generic parameter T inside a GenericDecl shadows a same-named decl in the enclosing scope.                                  | [`generic-param-shadows-outer-typedef.slang`](generic-param-shadows-outer-typedef.slang)                      |
| L-21     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes)                                   | Ambiguous reference: two value decls of the same name reachable via two using-namespaces produce `ambiguous-reference` (39999).| [`ambiguous-reference-from-two-using-namespaces.slang`](ambiguous-reference-from-two-using-namespaces.slang)            |
| L-22     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes)                                   | Forward use inside a block sees hiddenFromLookup; lookup falls through to the outer same-name decl rather than the inner.    | [`forward-use-with-outer-resolves-to-outer.slang`](forward-use-with-outer-resolves-to-outer.slang)                 |
| L-23     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes)                                   | Member lookup on ErrorType silently returns empty; only the original undefined-identifier diag fires, no cascading "no member".| [`member-lookup-on-error-type-silent.slang`](member-lookup-on-error-type-silent.slang)                       |
| L-24     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes)                                   | `IgnoreForLookupModifier` on an enum's tag-type base hides the integer-side members from the enum's member-lookup surface.    | [`enum-tag-type-base-not-surfaced.slang`](enum-tag-type-base-not-surfaced.slang)                          |

## Tests in this bundle

| File                                                              | Intent     | Doc anchor                                              |
| ----------------------------------------------------------------- | ---------- | ------------------------------------------------------- |
| [`agg-type-this-rewrite-from-nested-block.slang`](agg-type-this-rewrite-from-nested-block.slang)                   | functional | `#unqualified-lookup`                                   |
| [`ambiguous-reference-from-two-using-namespaces.slang`](ambiguous-reference-from-two-using-namespaces.slang)             | negative   | `#edge-cases-and-failure-modes`                         |
| [`breadcrumb-this-implicit-in-method.slang`](breadcrumb-this-implicit-in-method.slang)                        | functional | `#breadcrumbs`                                          |
| [`container-overload-accumulates-struct-methods.slang`](container-overload-accumulates-struct-methods.slang)             | functional | `#container-level-overload-accumulation`                |
| [`enum-tag-type-base-not-surfaced.slang`](enum-tag-type-base-not-surfaced.slang)                           | negative   | `#edge-cases-and-failure-modes`                         |
| [`extension-and-direct-member-merge-into-overload-set.slang`](extension-and-direct-member-merge-into-overload-set.slang)       | functional | `#member-lookup`                                        |
| [`forward-use-with-outer-resolves-to-outer.slang`](forward-use-with-outer-resolves-to-outer.slang)                  | functional | `#edge-cases-and-failure-modes`                         |
| [`generic-param-shadows-outer-typedef.slang`](generic-param-shadows-outer-typedef.slang)                       | functional | `#generic-parameters`                                   |
| [`inheritance-walk-traverses-multi-level-chain.slang`](inheritance-walk-traverses-multi-level-chain.slang)              | functional | `#member-lookup`                                        |
| [`interface-override-wins-over-default.slang`](interface-override-wins-over-default.slang)                      | functional | `#interface-requirements-vs-default-implementations`    |
| [`member-lookup-on-error-type-silent.slang`](member-lookup-on-error-type-silent.slang)                        | negative   | `#edge-cases-and-failure-modes`                         |
| [`member-lookup-resolves-direct-field.slang`](member-lookup-resolves-direct-field.slang)                       | functional | `#member-lookup`                                        |
| [`member-lookup-walks-interface-inheritance.slang`](member-lookup-walks-interface-inheritance.slang)                 | functional | `#member-lookup`                                        |
| [`member-lookup-walks-struct-inheritance.slang`](member-lookup-walks-struct-inheritance.slang)                    | functional | `#member-lookup`                                        |
| [`member-not-found-diagnostic.slang`](member-not-found-diagnostic.slang)                               | negative   | `#member-lookup`                                        |
| [`namespace-merge-via-using.slang`](namespace-merge-via-using.slang)                                 | functional | `#module-and-namespace`                                 |
| [`namespace-reopened-then-using-injects-all.slang`](namespace-reopened-then-using-injects-all.slang)                 | functional | `#module-and-namespace`                                 |
| [`pointer-like-auto-deref-constant-buffer.slang`](pointer-like-auto-deref-constant-buffer.slang)                   | functional | `#member-lookup`                                        |
| [`this-breadcrumb-mutating-mode.slang`](this-breadcrumb-mutating-mode.slang)                             | functional | `#unqualified-lookup`                                   |
| [`this-breadcrumb-static-type-mode.slang`](this-breadcrumb-static-type-mode.slang)                          | functional | `#unqualified-lookup`                                   |
| [`transparent-cbuffer-resolves-field.slang`](transparent-cbuffer-resolves-field.slang)                        | functional | `#transparent-members`                                  |
| [`unqualified-functions-accumulate-across-scopes.slang`](unqualified-functions-accumulate-across-scopes.slang)            | functional | `#unqualified-lookup`                                   |
| [`unqualified-short-circuits-on-variable.slang`](unqualified-short-circuits-on-variable.slang)                    | functional | `#unqualified-lookup`                                   |
| [`unqualified-walks-parent-chain.slang`](unqualified-walks-parent-chain.slang)                            | functional | `#unqualified-lookup`                                   |

## Sibling-bundle overlap

`tests-agentic/name-resolution/scopes/` covers the **boundaries** of
scopes (where a name is or is not visible). This bundle covers **how
the lookup algorithm resolves** once started. Claims intentionally
not duplicated here:

- "block-local name not visible outside" (scopes C-01) — scopes owns
  it. Our forward-use test angles differently: it shows the inner
  hidden local does not preempt an outer same-name decl.
- "generic parameter visible in body" (scopes C-13) — scopes owns
  the positive visibility. Our test angles differently: it shows
  the generic param **shadows** a same-name outer decl.
- "namespace reopened merges members" (scopes C-12) — scopes owns
  the qualified `Foo::a()` + `Foo::b()` case. Our two tests cover
  the *unqualified* `using namespace` cases instead.
- "extension method callable via receiver" (scopes C-17) — scopes
  owns the basic case. Our extension test covers the *overload-set
  merge* between direct members and extension members.

## Doc gaps observed

- The `## Concepts` section lists `LookupMask` and its bits in
  detail (type / Function / Value / Attribute / SyntaxDecl /
  Semantic / Default), but Slang's surface language does not let a
  user share names between a struct and a function (the "conflicting
  declaration" diagnostic at slang-diagnostics.lua code 30200
  fires); the documented mask-separation behavior is internal to
  the parser/checker. A user-observable claim about mask filtering
  would be helpful — e.g. naming an input shape where two same-name
  decls of different mask kinds **do** survive past redeclaration
  checks and the mask actually picks between them.
- The `## Concepts` section enumerates `LookupOptions` flags
  (`IgnoreBaseInterfaces`, `Completion`, `NoDeref`, etc.) but only
  `IgnoreInheritance` and `NoDeref` have indirect user surfaces
  (extension-on-same-type stays in scope; `ExtensionDecl` forces
  NoDeref for `This`). `Completion` and `ConsiderAllLocalNamesInScope`
  are language-server- and parser-only and not exercisable through
  `slang-test`. A doc-gap note could clarify which options have
  any user-observable consequence.
- The `## Algorithm > Transparent members` section says the
  recursion is short-circuited "when the request's `mask` includes
  `Attribute`" to avoid infinite recursion on attribute targets,
  but there is no user-observable way to issue an attribute-mask
  lookup from a `.slang` source. The negative case for this
  short-circuit (i.e. "an attribute-targeted lookup does NOT see
  through transparent members") cannot be anchored from a user
  source file. A claim about the attribute decl recursion that
  shows on the surface (e.g. through a synthesized attribute
  decl example) would unblock a test.
- The `## Algorithm > Transparent members` section names the
  `IgnoreTransparentMembers` option as used by "looking up an
  unscoped-enum's underlying type to break a similar cycle"; the
  doc does not name a user-surface that toggles this option. A
  positive test for this option would have to anchor to an
  unstated claim, so it is deferred.
- The `## Algorithm > Member lookup` section names `EachType` /
  `FirstPackElementType` / `LastPackElementType` / `PackBranchType`
  canonicalization in the type-shape dispatch, but these arise
  only inside variadic-pack contexts (no exposed surface in
  user `.slang` outside specific generic-pack idioms). A test
  could be added if the doc highlighted a specific user pattern
  that takes that path.
- The `## Edge cases and failure modes` section names
  `ExtensionExternVarModifier` and `ExternModifier` in extensions
  as filtered at the start of `DeclPassesLookupMask`. Both
  modifiers are core-module-only; user code cannot apply them to
  an extension to produce the rejection. A claim that some
  user-writable form maps to those modifiers would unblock a
  negative test.
- The `## Edge cases and failure modes` section says `AndType`
  reaching the type dispatch is an internal-error
  (`SLANG_UNEXPECTED`). That is a compiler-developer claim
  ("constraint-flattening must run before lookup"), not a user-
  observable claim, so no test anchors here.

## Out of scope (no-GPU runner)

None of the lookup claims in this bundle require a GPU.
`transparent-cbuffer-resolves-field.slang` and
`pointer-like-auto-deref-constant-buffer.slang` both compile to
HLSL text and FileCheck the emitted code; no driver runtime is
invoked.
