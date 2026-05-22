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


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| When an unqualified name inside a method resolves to a field of the enclosing struct, lookup records a `This` breadcrumb so the synthesized expression reads `this.field`. | functional | [#breadcrumbs](../../../docs/llm-generated/name-resolution/lookup.md#breadcrumbs) | [`breadcrumb-this-implicit-in-method.slang`](breadcrumb-this-implicit-in-method.slang) |
| Same-named methods inside one struct container accumulate into a LookupResult via the _prevInContainerWithSameName chain; overload resolution then picks the candidate matching the argument type. | functional | [#container-level-overload-accumulation](../../../docs/llm-generated/name-resolution/lookup.md#container-level-overload-accumulation) | [`container-overload-accumulates-struct-methods.slang`](container-overload-accumulates-struct-methods.slang) |
| Member lookup on an ErrorType silently returns empty so a downstream member access does not cascade into a 'member not found' diagnostic. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | [`member-lookup-on-error-type-silent.slang`](member-lookup-on-error-type-silent.slang) |
| The synthetic tag-type inheritance on enums carries IgnoreForLookupModifier so the underlying integer type's members are not surfaced as enum members; access to such a member produces 'member not found'. | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | [`enum-tag-type-base-not-surfaced.slang`](enum-tag-type-base-not-surfaced.slang) |
| When a name is referenced before its DeclStmt inside a block, the inner local is hiddenFromLookup at that point; lookup falls through to an outer same-name decl rather than resolving the inner. | functional | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | [`forward-use-with-outer-resolves-to-outer.slang`](forward-use-with-outer-resolves-to-outer.slang) |
| When a non-overloadable lookup resolves to two same-named value decls reachable via two sibling-injected namespaces, the checker emits ambiguous-reference (code 39999). | negative | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | [`ambiguous-reference-from-two-using-namespaces.slang`](ambiguous-reference-from-two-using-namespaces.slang) |
| A generic parameter T shadows a same-named decl in an enclosing scope because the GenericDecl scope is the closer parent during the unqualified-lookup walk. | functional | [#generic-parameters](../../../docs/llm-generated/name-resolution/lookup.md#generic-parameters) | [`generic-param-shadows-outer-typedef.slang`](generic-param-shadows-outer-typedef.slang) |
| A conforming-type override of an interface requirement shadows the interface's default implementation; the override is what executes. | functional | [#interface-requirements-vs-default-implementations](../../../docs/llm-generated/name-resolution/lookup.md#interface-requirements-vs-default-implementations) | [`interface-override-wins-over-default.slang`](interface-override-wins-over-default.slang) |
| Member lookup on a DeclRefType delegates to the inheritance walk, so a field declared on a base struct is reachable through a derived value. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`member-lookup-walks-struct-inheritance.slang`](member-lookup-walks-struct-inheritance.slang) |
| Member lookup on a pointer-like ConstantBuffer<T> auto-dereferences to the pointee and finds the inner field through a Deref breadcrumb. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`pointer-like-auto-deref-constant-buffer.slang`](pointer-like-auto-deref-constant-buffer.slang) |
| Member lookup that exhausts the inheritance walk without finding a candidate produces the 'member not found' diagnostic (code 30027), naming both the missing identifier and the host type. | negative | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`member-not-found-diagnostic.slang`](member-not-found-diagnostic.slang) |
| Member lookup walks the inheritance facets including extensions; direct members and same-name extension members of the same type accumulate into one overload set. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`extension-and-direct-member-merge-into-overload-set.slang`](extension-and-direct-member-merge-into-overload-set.slang) |
| Member lookup walks the inheritance facets to reach a method declared on an interface; the conforming struct's override is callable through the struct receiver directly. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`member-lookup-walks-interface-inheritance.slang`](member-lookup-walks-interface-inheritance.slang) |
| The inheritance walk traverses every facet of the InheritanceInfo, including transitive super-interfaces, so a method declared on a top-most interface is reachable through a deeply derived constraint. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`inheritance-walk-traverses-multi-level-chain.slang`](inheritance-walk-traverses-multi-level-chain.slang) |
| lookUpMember on a DeclRefType resolves a direct field of the type via the underlying decl. | functional | [#member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#member-lookup) | [`member-lookup-resolves-direct-field.slang`](member-lookup-resolves-direct-field.slang) |
| A UsingDecl captures the current scope and injects the named namespace into the sibling-scope chain so the namespace's unqualified names become reachable at the use site. | functional | [#module-and-namespace](../../../docs/llm-generated/name-resolution/lookup.md#module-and-namespace) | [`namespace-merge-via-using.slang`](namespace-merge-via-using.slang) |
| Multiple namespace blocks of the same name collapse into the same NamespaceDecl; a single `using namespace` then injects every accumulated member into the unqualified lookup chain. | functional | [#module-and-namespace](../../../docs/llm-generated/name-resolution/lookup.md#module-and-namespace) | [`namespace-reopened-then-using-injects-all.slang`](namespace-reopened-then-using-injects-all.slang) |
| An HLSL cbuffer is lowered to a __transparent ConstantBuffer; an unqualified reference to its inner field resolves through the transparent (Member) + deref breadcrumb chain. | functional | [#transparent-members](../../../docs/llm-generated/name-resolution/lookup.md#transparent-members) | [`transparent-cbuffer-resolves-field.slang`](transparent-cbuffer-resolves-field.slang) |
| A static member function carries a `This` breadcrumb that resolves enclosing static members through the type (not through a value instance), so unqualified `factor` reads the static field. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`this-breadcrumb-static-type-mode.slang`](this-breadcrumb-static-type-mode.slang) |
| Functions are overloadable, so step 6 does NOT short-circuit on a function hit; the lookup walks outward and a same-name function in an enclosing namespace remains a candidate. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`unqualified-functions-accumulate-across-scopes.slang`](unqualified-functions-accumulate-across-scopes.slang) |
| Step 4 of unqualified-lookup detects AggTypeDeclBase and rewrites the request to member lookup with a This breadcrumb; this rewrite still applies when the use site is several block scopes deep inside the method body. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`agg-type-this-rewrite-from-nested-block.slang`](agg-type-this-rewrite-from-nested-block.slang) |
| Step 5 of the unqualified-lookup walk updates ThisParameterMode based on the enclosing function's modifiers; [mutating] yields a mutable This breadcrumb so unqualified writes to a field in the method body update the receiver. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`this-breadcrumb-mutating-mode.slang`](this-breadcrumb-mutating-mode.slang) |
| Step 6 short-circuits on a non-overloadable hit, so an inner local variable stops the outward scope walk and a same-named outer value is never seen. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`unqualified-short-circuits-on-variable.slang`](unqualified-short-circuits-on-variable.slang) |
| The unqualified-lookup outer loop walks request.scope->parent chain to the module root, so an inner block resolves a function-scope parameter through the enclosing function scope. | functional | [#unqualified-lookup](../../../docs/llm-generated/name-resolution/lookup.md#unqualified-lookup) | [`unqualified-walks-parent-chain.slang`](unqualified-walks-parent-chain.slang) |


## Untested claims
NA


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#concepts](../../../docs/llm-generated/name-resolution/lookup.md#concepts) | undocumented-behavior | The `## Concepts` section lists `LookupMask` and its bits in detail (type / Function / Value / Attribute / SyntaxDecl / Semantic / Default), but Slang's surface language does not let a user share names between a struct and a function (the "conflicting declaration" diagnostic at slang-diagnostics.lua code 30200 fires); the documented mask-separation behavior is internal to the parser/checker. A user-observable claim about mask filtering would be helpful — e.g. naming an input shape where two same-name decls of different mask kinds **do** survive past redeclaration checks and the mask actually picks between them. |  |
| [#concepts](../../../docs/llm-generated/name-resolution/lookup.md#concepts) | undocumented-behavior | The `## Concepts` section enumerates `LookupOptions` flags (`IgnoreBaseInterfaces`, `Completion`, `NoDeref`, etc.) but only `IgnoreInheritance` and `NoDeref` have indirect user surfaces (extension-on-same-type stays in scope; `ExtensionDecl` forces NoDeref for `This`). `Completion` and `ConsiderAllLocalNamesInScope` are language-server- and parser-only and not exercisable through `slang-test`. A doc-gap note could clarify which options have any user-observable consequence. |  |
| [#algorithm-transparent-members](../../../docs/llm-generated/name-resolution/lookup.md#algorithm-transparent-members) | undocumented-behavior | The `## Algorithm > Transparent members` section says the recursion is short-circuited "when the request's `mask` includes `Attribute`" to avoid infinite recursion on attribute targets, but there is no user-observable way to issue an attribute-mask lookup from a `.slang` source. The negative case for this short-circuit (i.e. "an attribute-targeted lookup does NOT see through transparent members") cannot be anchored from a user source file. A claim about the attribute decl recursion that shows on the surface (e.g. through a synthesized attribute decl example) would unblock a test. |  |
| [#algorithm-transparent-members](../../../docs/llm-generated/name-resolution/lookup.md#algorithm-transparent-members) | undocumented-behavior | The `## Algorithm > Transparent members` section names the `IgnoreTransparentMembers` option as used by "looking up an unscoped-enum's underlying type to break a similar cycle"; the doc does not name a user-surface that toggles this option. A positive test for this option would have to anchor to an unstated claim, so it is deferred. |  |
| [#algorithm-member-lookup](../../../docs/llm-generated/name-resolution/lookup.md#algorithm-member-lookup) | undocumented-behavior | The `## Algorithm > Member lookup` section names `EachType` / `FirstPackElementType` / `LastPackElementType` / `PackBranchType` canonicalization in the type-shape dispatch, but these arise only inside variadic-pack contexts (no exposed surface in user `.slang` outside specific generic-pack idioms). A test could be added if the doc highlighted a specific user pattern that takes that path. |  |
| [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | undocumented-behavior | The `## Edge cases and failure modes` section names `ExtensionExternVarModifier` and `ExternModifier` in extensions as filtered at the start of `DeclPassesLookupMask`. Both modifiers are core-module-only; user code cannot apply them to an extension to produce the rejection. A claim that some user-writable form maps to those modifiers would unblock a negative test. |  |
| [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes) | undocumented-behavior | The `## Edge cases and failure modes` section says `AndType` reaching the type dispatch is an internal-error (`SLANG_UNEXPECTED`). That is a compiler-developer claim ("constraint-flattening must run before lookup"), not a user- observable claim, so no test anchors here. |  |


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
