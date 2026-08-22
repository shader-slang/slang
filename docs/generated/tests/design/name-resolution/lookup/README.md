---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-13T00:00:00+00:00
source_commit: c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
watched_paths_digest: b64dd978a4791dc55a97d3e798586d15994831751b1de432433826ab15c5313e
source_doc: docs/generated/design/name-resolution/lookup.md
source_doc_digest: ba75d93a4211c8edc3185687699c525c8793a39baf2b90b7166575aebfca06ab
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/name-resolution/lookup

## Intent

This bundle exercises the lookup algorithm described in
[`docs/generated/design/name-resolution/lookup.md`](../../../../design/name-resolution/lookup.md):
the outward scope walk and its short-circuit rule, the aggregate-type
rewrite and the this-parameter-mode updates it performs, the type
dispatch and facet walk of member lookup, pointer-like
auto-dereference, transparent-member injection, the navigation
(breadcrumb) chain and the optional-constraint filter that reads it,
block-local hiding, container-level overload accumulation, the
deliberate absence of cross-path deduplication, namespace merging and
sibling wiring, override-beats-default for interface defaults, the
keyword/identifier namespace sharing, generic-parameter shadowing, and
the documented failure modes (ambiguous reference, member not found,
error-type silence, enum tag-type exclusion, forward reference inside a
block).

Coverage strategy: one claim per doc section arm, with a positive
`//TEST:INTERPRET` test that echoes the resolved value for each
algorithm-shape claim and a `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`
test that pins the diagnostic for each failure mode. Boundary probes
sit on the same anchor as their parent claim: inheritance depth three,
a diamond base, the second pointer-like resource shape, the second
buffer-block keyword, the other two modifier spellings, the type half
of the short-circuit rule, and the constructor/setter arms of the
this-parameter-mode update.

Multi-backend rule: lookup is target-independent — the resolved decl
and its diagnostics are fixed at semantic check, before any backend
lowering — so each claim carries a single directive. The six tests
that use `-target hlsl` do so only because their surface
(`cbuffer` / `tbuffer`, `ConstantBuffer`/`ParameterBlock` globals, an
entry-point attribute, a system-value semantic) cannot be hosted by the
interpreter, and the one `COMPARE_COMPUTE -cpu` test does so because
the interpreter does not model exception semantics — not to add
per-target coverage.

## Claims

Enumerated per [`_claims.md` §1](../../../_meta/prompts/_claims.md).
This page is largely about internal compiler structure — the
`LookupResult` / `LookupRequest` types, the `LookupMask` and
`LookupOptions` bitsets, the breadcrumb records, and the linearized
facet list — so the enumeration is **split**: C1–C67 are claims with a
user-observable consequence in Slang source, and C68–C101 are
internal-source facts that a `.slang` input cannot distinguish, which
are routed to `## Untested claims` with reason `internal-source-fact`
(or `needs-unit-test` where a C++ unit test would reach them).

### User-observable claims

**Concepts**

1. `declToExclude` exists so that a declaration being checked cannot find itself — the header's example is `typedef Foo Foo;`.
2. `AttributeDecl`s are the declarations introduced by `attribute_syntax [name(...)]` in the core module, and source reaches them only by writing `[name(...)]` on a declaration.
3. `SemanticDecl`s are the `semantic sv_position { ... }` declarations that record which types and stages a `: SV_*` annotation on a shader parameter is valid for.
4. The parser asks for the `SyntaxDecl` bit on its own when reading the modifier list of a parameter declaration, so that only a keyword decl can begin a modifier there.
5. `Attribute` and `Semantic` are the two bits outside `Default`, so an ordinary identifier spelled like an attribute neither hides the attribute nor is hidden by it.
6. A use site that needs exactly one of `type` / `Function` / `Value` narrows the result after the fact through `refineLookup`, as the operand of `fwd_diff(...)` / `bwd_diff(...)` does with `LookupMask::Function`.

**Unqualified lookup**

7. The outer loop walks `request.scope` outward via the `parent` chain, in practice terminating at the module root.
8. At each scope the inner loop walks `nextSibling` so that sibling `NamespaceDecl`s, `using`-injected namespaces, and imported-module scopes are consulted at the same level.
9. When the container is an `AggTypeDeclBase` the request is rewritten to member lookup against the corresponding `Type*`, with a `This` breadcrumb recording the implicit `this`/`This` of the enclosing decl.
10. Otherwise the request falls through to `_lookUpDirectAndTransparentMembers` on that container.
11. `ConstructorDecl` and `SetterDecl` give a mutable `this`.
12. A `[mutating]`-qualified function leaves a mutable-value `this`.
13. An effectively-static member function, and stepping out of a nested `AggTypeDeclBase`, leave only the `This` type rather than a `this` value.
14. `[ref]` is the third modifier a `FunctionDeclBase` consults when updating `thisParameterMode`.
15. After visiting one scope and its siblings, lookup stops walking outwards when the result is valid and is neither overloaded nor overloadable.
16. Callables (and generics wrapping them) are overloadable so they continue to accumulate candidates from outer scopes; types and variables do not.
17. Outside completion mode a container is searched only for members whose name matches the request, through the same-name list.
18. After direct members, transparent direct members are searched and a `Member` breadcrumb is recorded.

**Member lookup**

19. Member lookup on a plain `DeclRefType` resolves a direct member of the underlying type decl.
20. A `DeclRefType` delegates to the facet walk, so a member contributed by a base type is reachable through a derived value.
21. `ModifiedType` — modifiers are transparent to lookup; the facet walk runs on the modified type directly.
22. Exactly three spellings produce a `ModifiedType` — `unorm`, `snorm`, and `no_diff` — and the parser moves such a modifier off the declaration and onto its type expression, so a member access on a value declared `no_diff S` reaches this arm.
23. `ExtractExistentialType` — a variable, parameter, or field declared with an interface type produces one, and the implicit `ThisType` decl-ref of that interface is the target of lookup.
24. Anything else — including `ErrorType` — falls off the end of the dispatch chain and contributes no items.
25. The name `This` in anything other than an `InterfaceDecl` resolves to the decl-ref itself.
26. The facet walk skips non-`Self` facets when `IgnoreInheritance` is set, with a special case that keeps an `extension` whose target type equals the self type.
27. The facet walk skips facets whose `subtypeWitness` comes from an inheritance decl carrying `IgnoreForLookupModifier` — the synthetic tag-type inheritance on `enum`s.
28. An inherited `This` candidate is suppressed entirely when the self type is an interface.
29. For a non-`Self` facet of `Facet::Kind::Type`, a `SuperType` breadcrumb carrying the `subtypeWitness` is prepended.
30. Members found through an interface facet come back already specialized to the concrete conforming type, because the facet's parent decl-ref is a `LookupDeclRef` over the interface's `ThisTypeDecl` and the witness.
31. The linearized facet list is transitive, so a member declared on a super-interface of a super-interface is still reachable.
32. A pointer-like type is auto-dereferenced before the per-type dispatch: a `Deref` breadcrumb is prepended, lookup recurses on the pointee, and a valid result short-circuits the rest of the dispatch.
33. `InterfaceDefaultImplDecl` triggers a separate path that looks up in the explicit `This` parameter instead of the interface itself.

**Transparent members**

34. A `TransparentModifier` on a member of a `ContainerDecl` causes its own members to be searched whenever the parent is searched.
35. HLSL `cbuffer C { float4 f; }` lowers to an anonymous struct plus a transparent `ConstantBuffer` of it, so an unqualified reference to `f` resolves through `anon1.f`.
36. `__transparent` is not a source-writable modifier — no keyword in the parser's tables introduces one.
37. The single site that creates a `TransparentModifier` backs the `cbuffer` and `tbuffer` declaration keywords and the GLSL interface-block forms, so every transparent member in a user program comes from one of those buffer-block declarations.

**Breadcrumbs**

38. For the `cbuffer` example the chain is `Member` then `Deref`, so an unqualified `f` becomes the equivalent of `(*anon1).f`.
39. For an unqualified `g` defined on `Self` inside a method the breadcrumb is just `This`, marking that the rewritten expression needs an implicit `this.g`.
40. For lookup through an interface base the breadcrumb is `SuperType` carrying the witness as its `val`, and `filterLookupResultByCheckedOptional` drops an item when any witness on the chain comes from an `optional` constraint the surrounding code has not checked.
41. The witness counts as checked only inside an enclosing `if` whose predicate is an `is` test naming the same sub- and super-type, so the doc's `g` is accepted where its `f` is rejected.

**Block-local shadowing**

42. Inside a `BlockStmt` every declaration is hidden from lookup until the checker walks past its own `DeclStmt`.
43. A use of the name from a nested block textually before the declaration is therefore undefined.
44. `_isUncheckedLocalVar` suppresses a decl only when `isLocalVar` holds for it.
45. `visitCatchStmt` clears the hidden flag on a `catch` clause's error variable.

**Container-level overload accumulation**

46. Decls with the same name inside one `ContainerDecl` do not shadow each other; they accumulate into a `LookupResult` and become an overload set.
47. The accelerator rebuild deliberately skips a `GenericDecl`'s `inner` member so that a generic and its inner decl do not both answer to the generic's name.

**Deduplication**

48. Lookup can return competing items for different declarations, such as an interface requirement alongside the concrete member that satisfies it, and the caller's narrowing is where the concrete member wins.
49. Within member lookup the facet list is deduplicated by origin, so a base reached through several inheritance paths contributes one facet and its members appear once.

**Module and namespace**

50. Multiple `namespace Foo {}` declarations under the same parent container parse into the same `NamespaceDecl`.
51. The target of a `using` declaration is attached to the scope chain as a sibling, so its names become reachable unqualified.
52. Wiring completes before any signature is checked, so a `using` declaration may appear after the declaration whose header depends on it and the header still resolves.
53. Same-named namespaces in different `FileDecl`s of one module, and in different modules, are attached as siblings rather than merged.
54. `importModuleIntoScope` filters what an `import` re-exports through `isOwnModuleOrIncludedFileScope`.

**Interface requirements vs default implementations**

55. A user-provided extension member can shadow an interface default implementation.
56. Lookup from inside a default implementation advances the scope cursor past every scope up to and including the enclosing `InterfaceDecl`, so the interface's own requirement declarations are never consulted and a witness override on a conforming type wins over the default.

**Keyword vs identifier**

57. Keywords are `SyntaxDecl`s registered in the core module that share the identifier namespace with user decls, so shadowing is decided by ordinary lookup rather than by a separate keyword table.
58. `tryLookUpSyntaxDecl` rejects the result unless the single found decl is a `SyntaxDecl`, so a local of the same name wins.
59. `isKeywordAvailable` treats a contextual keyword as available only when plain lookup of that identifier finds nothing at all, so any user declaration of the name disables the keyword.

**Generic parameters**

60. A reference to `T` from inside the generic's inner decl resolves through the `GenericDecl` scope, shadowing any same-named decl in the enclosing scope.
61. A reference to `T` from a sibling of the outer decl never finds the generic parameter.

**Edge cases and failure modes**

62. A `LookupResult` with multiple items matching the mask is returned as overloaded, with ranking deferred to overload resolution.
63. `refineLookup` drops every item failing `DeclPassesLookupMask` silently and returns the single survivor, with no diagnostic for the filtered-out candidates.
64. When narrowing leaves more than one candidate and the context needs exactly one, `AmbiguousReference` (code 39999) is emitted followed by one candidate note per surviving item.
65. A `NamespaceDecl` first item is exempted from that report because an overloaded namespace reference is legitimate.
66. A forward reference inside a `BlockStmt` is skipped by `_isUncheckedLocalVar`, so lookup returns the outer decl or empty and `UndefinedIdentifier` takes over.
67. Lookup itself never diagnoses, so an empty member-lookup result is turned into the member-not-found diagnostic by its caller.

### Internal-source facts

68. Four entry points are declared in `slang-lookup.h`: `lookUp` (unqualified, folding two `bool`s into `LookupOptions`), `lookUpMember` (qualified, taking a full `LookupOptions`), `lookUpDirectAndTransparentMembers` (one container, always `LookupOptions::None`), and `refineLookup` (a post-filter).
69. `AddToLookupResult` is exported in an item-at-a-time and a merge-a-whole-result overload so callers outside `slang-lookup.cpp` accumulate items the same way.
70. `LookupRequest` bundles `semantics`, `scope`, `endScope`, `declToExclude`, `mask`, and `options` plus two predicates, and `initLookupRequest` auto-sets `Completion` when the name matches the session's completion token.
71. `endScope` is never assigned by any caller, so the scope walk always runs until the parent chain reaches null.
72. `request.semantics` may be null for a lookup performed from the parser, which changes behaviour in unqualified step 4 and in member lookup.
73. The `LookupMask` bit values are `type = 0x1`, `Function = 0x2`, `Value = 0x4`, `Attribute = 0x8`, `SyntaxDecl = 0x10`, `Semantic = 0x20`, and `Default = type | Function | Value | SyntaxDecl`.
74. `Value` is the fall-through category: everything that is neither a type, a function, an attribute, a syntax decl, nor a semantic decl.
75. In `DeclPassesLookupMask` the `extern`-related rejections run before any bit is consulted, and `FileDecl` is hard-coded never to pass.
76. `LookupOptions` has six flags: `IgnoreBaseInterfaces`, `Completion`, `NoDeref`, `ConsiderAllLocalNamesInScope`, `IgnoreInheritance`, and `IgnoreTransparentMembers`.
77. `Completion` returns every applicable decl in a container rather than only same-named ones and does not stop the outward walk on the first hit.
78. `ConsiderAllLocalNamesInScope` bypasses the `hiddenFromLookup` / check-state test, and its one caller is `tryLookUpSyntaxDecl`, which also passes a null `SemanticsVisitor`.
79. `LookupResultItem` is one found decl plus an optional breadcrumb chain and exposes `Breadcrumb` as a nested typedef.
80. The breadcrumb `Kind` enum has exactly four values — `Member`, `Deref`, `SuperType`, `This` — and instances chain through `next` carrying a `ThisParameterMode` of `ImmutableValue`, `MutableValue`, or the `This` `Type`.
81. A `LookupResult` is valid when the item's decl is non-null and overloaded when `items.getCount() > 1`; when `items` is in use it holds all the items and `item` duplicates the first, and `items` stays empty in the single-result case so no heap allocation happens.
82. A facet is a `(kind, directness, origin, subtypeWitness, declRefForMemberLookup)` record whose `kind` is `Type` or `Extension` and whose `directness` is `Self` (0), `Direct` (1), or a larger indirection count.
83. The scope walk skips a null `containerDecl` sentinel and skips a `FileDecl` it has already visited on the same chain.
84. Each `containerDecl` is first turned into a `DeclRef` by `createDefaultSubstitutionsIfNeeded`.
85. In the named-lookup branch the check-state test additionally requires `request.semantics` to be non-null, so a parse-time lookup never suppresses a local on check-state grounds.
86. `DeclPassesLookupMask` drops decls carrying `ExtensionExternVarModifier` and rejects `ExternModifier`-tagged members of `extension`s unconditionally.
87. Transparent-member recursion is skipped when the request's mask includes `Attribute` (a cycle breaker) or when `IgnoreTransparentMembers` is set, whose one context is the check of a transparent declaration's own base clause.
88. `getTransparentDirectMemberDecls` returns a cached list of direct members carrying `TransparentModifier`, populated when the container's lookup accelerators are rebuilt.
89. `_lookUpMembersInSuperTypeDeclImpl` drives the decl to `DeclCheckState::ReadyForLookup` and keys `getInheritanceInfo` on the `ExtensionDecl` decl-ref for an extension and on the canonical self type otherwise.
90. The facet walk skips facets with no `ContainerDecl` decl-ref, facets missing either a type or a `subtypeWitness`, and interface facets when `IgnoreBaseInterfaces` is set.
91. The facet walk calls `_lookUpDirectAndTransparentMembers` using `facet->declRefForMemberLookup` as the parent decl-ref.
92. The pack-element arms (`EachType`, `FirstPackElementType`, `LastPackElementType`, `PackBranchType`) canonicalize the type before entering the facet walk and return early when `request.semantics` is null.
93. `AndType` reaching the type dispatch is a `SLANG_UNEXPECTED`, because `visitGenericTypeConstraintDecl` should have flattened it earlier.
94. A member lookup with no semantics context sees only direct members of an `AggTypeDeclBase`.
95. `_lookUpInScopes` forces `NoDeref` when the enclosing scope is an `ExtensionDecl`, so the extension's `This` refers to the extension target itself.
96. When the enclosing scope is an `InterfaceDecl` the lookup is rewritten through the interface's `ThisTypeDecl` and the `This` breadcrumb is suppressed.
97. `CreateLookupResultItem` reverses the on-stack breadcrumb chain so the final order matches the navigation order from the source expression to the found decl.
98. `Decl::_prevInContainerWithSameName` is populated lazily when a container's lookup accelerators are rebuilt, not by `addDirectMemberDecl`.
99. `AddToLookupResult` appends without comparing against previously collected items, so the same `DeclRef` reached both directly and through a transparent member appears twice with different breadcrumb chains.
100.  Scope wiring is a dedicated check phase (`ScopesWired`) sitting between `ModifiersChecked` and `SignatureChecked`, and the whole module is driven to it before any declaration advances past it.
101.  Lookup itself never diagnoses; every diagnostic named on this page is raised by a caller.

## Functional coverage

| Claim                                                                                                                                                                                                                         | Intent     | Anchor                                                                                                                                               | Tests                                                                                                                    |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| C42: A local is hidden from lookup until its own declaration statement is reached, so a reference earlier in the block binds to a same-named outer decl instead.                                                              | functional | [#block-local-shadowing](../../../../design/name-resolution/lookup.md#block-local-shadowing)                                                         | [`forward-use-with-outer-resolves-to-outer.slang`](forward-use-with-outer-resolves-to-outer.slang)                       |
| C43, C66: Block-local hiding reaches into nested inner blocks, so a use of the name from a nested block textually before the declaration is undefined (E30015).                                                               | negative   | [#block-local-shadowing](../../../../design/name-resolution/lookup.md#block-local-shadowing)                                                         | [`nested-block-forward-ref-rejected.slang`](nested-block-forward-ref-rejected.slang)                                     |
| C44: The hidden-until-declared rule applies only to local variables, so a struct declared later in the same block is still found by an earlier reference.                                                                     | boundary   | [#block-local-shadowing](../../../../design/name-resolution/lookup.md#block-local-shadowing)                                                         | [`block-local-struct-visible-before-decl.slang`](block-local-struct-visible-before-decl.slang)                           |
| C45: A catch clause's error variable is cleared of the block-local hidden flag before its handler is checked, so the handler body can refer to it.                                                                            | boundary   | [#block-local-shadowing](../../../../design/name-resolution/lookup.md#block-local-shadowing)                                                         | [`catch-clause-variable-visible-in-handler.slang`](catch-clause-variable-visible-in-handler.slang)                       |
| C40: A member reached only through an unchecked optional conformance is removed after lookup, and the empty result is reported as the required-constraint-is-not-checked diagnostic (E30403).                                 | negative   | [#breadcrumbs](../../../../design/name-resolution/lookup.md#breadcrumbs)                                                                             | [`optional-constraint-unchecked-member-rejected.slang`](optional-constraint-unchecked-member-rejected.slang)             |
| C41: A member reached through an optional conformance survives the post-lookup filter once the surrounding code has checked the constraint with an is statement.                                                              | functional | [#breadcrumbs](../../../../design/name-resolution/lookup.md#breadcrumbs)                                                                             | [`optional-constraint-member-visible-after-is-check.slang`](optional-constraint-member-visible-after-is-check.slang)     |
| C39: An unqualified name inside a method that names a field of the enclosing type records a This navigation step, so the expression behaves as if it were written this.field.                                                 | functional | [#breadcrumbs](../../../../design/name-resolution/lookup.md#breadcrumbs)                                                                             | [`breadcrumb-this-implicit-in-method.slang`](breadcrumb-this-implicit-in-method.slang)                                   |
| C2, C5: The Attribute category is not part of the default lookup mask, so a value declared with an attribute's spelling neither hides the attribute nor is hidden by it.                                                      | functional | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                                                                   | [`lookup-mask-attribute-vs-value-name.slang`](lookup-mask-attribute-vs-value-name.slang)                                 |
| C3, C5: The semantic category sits outside the default lookup mask, so a value declared with a system-value semantic's spelling neither hides the semantic annotation nor is hidden by it.                                    | functional | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                                                                   | [`semantic-name-not-hidden-by-value-decl.slang`](semantic-name-not-hidden-by-value-decl.slang)                           |
| C4: A parameter's modifier list is read with a keyword-only mask, so a same-named value declaration does not stop the keyword from being recognised there while the identifier still names the value in an expression.        | functional | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                                                                   | [`param-modifier-keyword-not-hidden-by-value.slang`](param-modifier-keyword-not-hidden-by-value.slang)                   |
| C1: The declaration being checked is excluded from its own lookup, so a typedef that reuses the name it is defined from resolves that name to the outer declaration.                                                          | functional | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                                                                   | [`typedef-self-name-excluded-from-lookup.slang`](typedef-self-name-excluded-from-lookup.slang)                           |
| C17, C46, C62: Same-named decls inside one container do not shadow each other; they accumulate through the same-name chain into one overload set and resolution picks by argument type.                                       | functional | [#container-level-overload-accumulation](../../../../design/name-resolution/lookup.md#container-level-overload-accumulation)                         | [`container-overload-accumulates-struct-methods.slang`](container-overload-accumulates-struct-methods.slang)             |
| C47: The same-name chain skips a generic's inner decl, so a generic and its inner declaration do not both answer to the generic's name and a same-named plain overload still resolves.                                        | boundary   | [#container-level-overload-accumulation](../../../../design/name-resolution/lookup.md#container-level-overload-accumulation)                         | [`generic-and-inner-decl-single-candidate.slang`](generic-and-inner-decl-single-candidate.slang)                         |
| C48: Lookup returns the interface requirements alongside the concrete member that satisfies them, and the caller's narrowing keeps only the concrete member so the call is not ambiguous.                                     | functional | [#deduplication](../../../../design/name-resolution/lookup.md#deduplication)                                                                         | [`concrete-member-beats-interface-requirements.slang`](concrete-member-beats-interface-requirements.slang)               |
| C49: The facet list is deduplicated by origin, so a base interface reached through two inheritance paths contributes one facet and its requirement is not ambiguous.                                                          | boundary   | [#deduplication](../../../../design/name-resolution/lookup.md#deduplication)                                                                         | [`diamond-base-requirement-single-facet.slang`](diamond-base-requirement-single-facet.slang)                             |
| C63, C6: The operand of fwd_diff is resolved against a function-only mask, so a same-named non-function candidate accumulated from an outer scope is dropped silently rather than reported as ambiguous.                      | functional | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes)                                           | [`fwd-diff-operand-refines-to-function-mask.slang`](fwd-diff-operand-refines-to-function-mask.slang)                     |
| C65: A result whose first item is a namespace is exempted from the ambiguity report, so a name reachable as two same-named namespaces stays usable as a qualifier.                                                            | boundary   | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes)                                           | [`overloaded-namespace-reference-not-ambiguous.slang`](overloaded-namespace-reference-not-ambiguous.slang)               |
| C24: Member lookup on the error type matches no arm of the type dispatch and returns empty silently, so a member access on an unresolved name does not cascade a second diagnostic.                                           | negative   | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes)                                           | [`member-lookup-on-error-type-silent.slang`](member-lookup-on-error-type-silent.slang)                                   |
| C27: The synthetic tag-type inheritance on an enum is filtered out of the facet walk, so the underlying integer type's members are not reachable as enum members (E30027).                                                    | negative   | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes)                                           | [`enum-tag-type-base-not-surfaced.slang`](enum-tag-type-base-not-surfaced.slang)                                         |
| C64: When narrowing leaves more than one candidate for a context that needs exactly one, the checker reports an ambiguous reference (E39999) with one candidate note per surviving item.                                      | negative   | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes)                                           | [`ambiguous-reference-from-two-using-namespaces.slang`](ambiguous-reference-from-two-using-namespaces.slang)             |
| C60: A generic parameter is a direct member of the generic's own scope, which the inner decl's parent chain passes through first, so it shadows a same-named decl in the enclosing scope.                                     | functional | [#generic-parameters](../../../../design/name-resolution/lookup.md#generic-parameters)                                                               | [`generic-param-shadows-outer-typedef.slang`](generic-param-shadows-outer-typedef.slang)                                 |
| C55: A conforming type's implementation of a requirement wins over the interface's default implementation of the same requirement.                                                                                            | functional | [#interface-requirements-vs-default-implementations](../../../../design/name-resolution/lookup.md#interface-requirements-vs-default-implementations) | [`interface-override-wins-over-default.slang`](interface-override-wins-over-default.slang)                               |
| C33, C56: Lookup from inside a default implementation goes through its explicit This parameter and skips the interface's own requirement decls, so a requirement it calls dispatches to the conforming type's implementation. | functional | [#interface-requirements-vs-default-implementations](../../../../design/name-resolution/lookup.md#interface-requirements-vs-default-implementations) | [`default-impl-calls-requirement-through-this.slang`](default-impl-calls-requirement-through-this.slang)                 |
| C57, C58, C59: A contextual keyword is only available when plain lookup of that identifier finds nothing, so a user local of the same spelling makes the parser treat the token as an ordinary identifier.                    | functional | [#keyword-vs-identifier](../../../../design/name-resolution/lookup.md#keyword-vs-identifier)                                                         | [`contextual-keyword-disabled-by-local-decl.slang`](contextual-keyword-disabled-by-local-decl.slang)                     |
| C21, C22: A type modifier is transparent to member lookup, so a field and a method of a value declared no_diff S resolve exactly as they do on the unmodified struct.                                                         | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`modified-type-no-diff-member-access.slang`](modified-type-no-diff-member-access.slang)                                 |
| C22: The other two modifier spellings that produce a modified type, unorm and snorm, are equally transparent to member lookup.                                                                                                | boundary   | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`modified-type-unorm-snorm-member-access.slang`](modified-type-unorm-snorm-member-access.slang)                         |
| C32: A ParameterBlock is pointer-like for lookup too, so a field of its element type is reachable through the same auto-dereference step.                                                                                     | boundary   | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`pointer-like-auto-deref-parameter-block.slang`](pointer-like-auto-deref-parameter-block.slang)                         |
| C30: A member found through an interface facet comes back already specialized to the concrete conforming type, so a requirement returning This yields the conformer type at the call site.                                    | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`interface-facet-declref-specialized-to-conformer.slang`](interface-facet-declref-specialized-to-conformer.slang)       |
| C28: An inherited This candidate is suppressed for an interface, so a derived interface that also inherits This from its base can still name This without an ambiguity.                                                       | boundary   | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`inherited-this-in-derived-interface-not-ambiguous.slang`](inherited-this-in-derived-interface-not-ambiguous.slang)     |
| C32: Member lookup auto-dereferences a pointer-like ConstantBuffer before the type dispatch, so a field of the pointee is reachable through a Deref navigation step.                                                          | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`pointer-like-auto-deref-constant-buffer.slang`](pointer-like-auto-deref-constant-buffer.slang)                         |
| C20, C29: Member lookup on a DeclRefType drives the facet walk, so a field declared on a base struct is reachable through a derived value.                                                                                    | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`member-lookup-walks-struct-inheritance.slang`](member-lookup-walks-struct-inheritance.slang)                           |
| C19: Member lookup on a plain DeclRefType resolves a direct field declared on the underlying type decl.                                                                                                                       | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`member-lookup-resolves-direct-field.slang`](member-lookup-resolves-direct-field.slang)                                 |
| C23: Member lookup on an interface-typed (existential) value targets the interface's implicit ThisType decl-ref, so the requirement is callable on the erased value.                                                          | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`existential-member-lookup-through-thistype.slang`](existential-member-lookup-through-thistype.slang)                   |
| C67: Member lookup that exhausts the facet walk without a candidate yields the member-not-found diagnostic (E30027) naming both the missing identifier and the host type.                                                     | negative   | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`member-not-found-diagnostic.slang`](member-not-found-diagnostic.slang)                                                 |
| C26: The facet walk keeps an extension whose target type equals the self type, so a direct member and a same-named extension member of that type accumulate into one overload set.                                            | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`extension-and-direct-member-merge-into-overload-set.slang`](extension-and-direct-member-merge-into-overload-set.slang) |
| C20: The facet walk reaches an interface facet of a conforming struct, and the struct's own implementation is what a direct receiver call resolves to.                                                                        | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`member-lookup-walks-interface-inheritance.slang`](member-lookup-walks-interface-inheritance.slang)                     |
| C31: The linearized facet list includes transitive super-interfaces, so a method declared on the topmost interface of a three-level chain is reachable through a constraint naming only the bottom one.                       | boundary   | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`inheritance-walk-traverses-multi-level-chain.slang`](inheritance-walk-traverses-multi-level-chain.slang)               |
| C25: The name This inside a non-interface aggregate type resolves to the enclosing type's own decl-ref rather than being looked up as an ordinary member.                                                                     | functional | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                                                                         | [`this-type-name-resolves-to-self-type.slang`](this-type-name-resolves-to-self-type.slang)                               |
| C50: Multiple same-named namespace blocks under one parent parse into a single namespace decl, so one using declaration makes members from every block reachable unqualified.                                                 | functional | [#module-and-namespace](../../../../design/name-resolution/lookup.md#module-and-namespace)                                                           | [`namespace-reopened-then-using-injects-all.slang`](namespace-reopened-then-using-injects-all.slang)                     |
| C52: Sibling-scope wiring runs as its own check phase before any signature is checked, so a declaration header resolves a name injected by a using declaration that appears later in the file.                                | boundary   | [#module-and-namespace](../../../../design/name-resolution/lookup.md#module-and-namespace)                                                           | [`using-wired-before-signature-check.slang`](using-wired-before-signature-check.slang)                                   |
| C8, C51: The target of a using declaration is attached to the scope chain as a sibling, so the namespace's names become reachable unqualified at the use site.                                                                | functional | [#module-and-namespace](../../../../design/name-resolution/lookup.md#module-and-namespace)                                                           | [`namespace-merge-via-using.slang`](namespace-merge-via-using.slang)                                                     |
| C18, C34, C35, C38: An HLSL cbuffer lowers to a transparent ConstantBuffer of an anonymous struct, so an unqualified reference to its inner field resolves through the transparent-member plus dereference navigation chain.  | functional | [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members)                                                             | [`transparent-cbuffer-resolves-field.slang`](transparent-cbuffer-resolves-field.slang)                                   |
| C36: No parser keyword introduces a transparent modifier, so writing \_\_transparent in front of a struct member is not recognised as a modifier and the declaration fails to parse.                                          | negative   | [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members)                                                             | [`transparent-modifier-not-source-writable.slang`](transparent-modifier-not-source-writable.slang)                       |
| C37: A tbuffer is the second buffer-block keyword that produces a transparent member, so an unqualified reference to its inner field resolves through the same transparent-plus-dereference navigation chain.                 | boundary   | [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members)                                                             | [`transparent-tbuffer-resolves-field.slang`](transparent-tbuffer-resolves-field.slang)                                   |
| C11: A constructor gives a mutable this, so an unqualified assignment to a field inside \_\_init initializes the object being constructed.                                                                                    | boundary   | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`constructor-this-breadcrumb-mutable.slang`](constructor-this-breadcrumb-mutable.slang)                                 |
| C11: A property setter gives a mutable this, so an unqualified assignment to a backing field inside the set accessor updates the receiver.                                                                                    | boundary   | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`setter-this-breadcrumb-mutable.slang`](setter-this-breadcrumb-mutable.slang)                                           |
| C15, C16: A type name is not overloadable either, so a block-local typedef stops the outward walk and the same-named file-scope struct is never accumulated.                                                                  | boundary   | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`unqualified-short-circuits-on-type.slang`](unqualified-short-circuits-on-type.slang)                                   |
| C15: A variable is not overloadable, so lookup stops the outward scope walk as soon as one is found and a same-named file-scope value is never accumulated.                                                                   | functional | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`unqualified-short-circuits-on-variable.slang`](unqualified-short-circuits-on-variable.slang)                           |
| C13: An effectively-static member function leaves only the This type rather than a this value, so an unqualified name in its body resolves to a static member of the enclosing type.                                          | functional | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`this-breadcrumb-static-type-mode.slang`](this-breadcrumb-static-type-mode.slang)                                       |
| C16: Callables are overloadable, so the outward scope walk does not short-circuit on a function hit and a same-named function in an enclosing scope stays a candidate.                                                        | functional | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`unqualified-functions-accumulate-across-scopes.slang`](unqualified-functions-accumulate-across-scopes.slang)           |
| C12: Stepping out of a mutating-qualified method sets the This parameter mode to a mutable value, so an unqualified write to a field inside that method updates the receiver.                                                 | functional | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`this-breadcrumb-mutating-mode.slang`](this-breadcrumb-mutating-mode.slang)                                             |
| C7, C10: The unqualified-lookup outer loop walks the scope parent chain outward to the module root, so a name inside a nested block resolves through the enclosing function scope and then the file scope.                    | functional | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`unqualified-walks-parent-chain.slang`](unqualified-walks-parent-chain.slang)                                           |
| C9: When the scope walk reaches an enclosing aggregate type it rewrites the request into member lookup with an implicit This breadcrumb, and that rewrite still fires from several block scopes deep inside a method body.    | boundary   | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                                                               | [`agg-type-this-rewrite-from-nested-block.slang`](agg-type-this-rewrite-from-nested-block.slang)                         |

## Untested claims

| Claim                                                                                                                                                                                                                                                          | Reason                | Anchor                                                                                                     | Why untested                                                                                                                                                                                                                                         |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | ---------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C14: `[ref]` is the third modifier a function declaration consults when updating the this-parameter mode.                                                                                                                                                      | implementation-detail | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                     | Writing `[ref]` on a method is reported as unknown attribute `E31000` and the body then fails on assignment, so the doc names no surface that selects this arm. Recorded as a doc gap below; the `[mutating]` and static arms are covered.           |
| C53: Same-named namespaces in different files of one module, and in different modules, are attached to the scope chain as siblings rather than merged into one namespace decl.                                                                                 | needs-multi-file-test | [#module-and-namespace](../../../../design/name-resolution/lookup.md#module-and-namespace)                 | Distinguishing the sibling attachment from the same-file merge needs two `.slang` files (a second `__include`d file, or a second module reached by `import`), which a single-file `//TEST` directive cannot express.                                 |
| C54: `importModuleIntoScope` filters what an `import` re-exports through `isOwnModuleOrIncludedFileScope`.                                                                                                                                                     | needs-multi-file-test | [#module-and-namespace](../../../../design/name-resolution/lookup.md#module-and-namespace)                 | The filter only has an effect on names arriving from an imported module, so a test needs at least a second module to import.                                                                                                                         |
| C61: A reference to a generic parameter from a sibling of the outer decl never finds it, because that scope chain does not pass through the generic decl.                                                                                                      | out-of-bundle         | [#generic-parameters](../../../../design/name-resolution/lookup.md#generic-parameters)                     | The negative visibility boundary is owned by the sibling `design/name-resolution/scopes/` bundle; this bundle covers the positive shadowing half (C60).                                                                                              |
| C68, C69, C70, C71, C79, C81, C82, C84, C88, C89, C91, C97, C98, C100: the entry-point inventory, the request/result/facet record shapes, and the accelerator and check-phase plumbing that build them.                                                        | internal-source-fact  | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                         | These name C++ signatures, struct fields, and cache-population points. Every observable consequence of them is already asserted through the resolutions in the coverage table; no `.slang` input distinguishes the internal shape that produced one. |
| C73, C74, C76, C80: the numeric `LookupMask` bit values and `Default` composition, the fall-through definition of the `Value` category, the six `LookupOptions` flags, and the four breadcrumb kinds with their this-parameter modes.                          | internal-source-fact  | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                         | The bit values and enumerator names are not reachable from Slang source; only the categories' behaviour is, and that behaviour is covered by C2–C5, C11–C13 and C38–C40.                                                                             |
| C77, C78: `Completion` returns every applicable decl in a container and does not stop the outward walk on the first hit, and `ConsiderAllLocalNamesInScope` bypasses the check-state shadowing test.                                                           | needs-unit-test       | [#concepts](../../../../design/name-resolution/lookup.md#concepts)                                         | Both options are set only by the language server and by the parser's own syntax-decl probe. No `slang-test` directive drives either entry point, so a C++ unit test issuing a completion-mode request is what would verify them.                     |
| C72, C85, C94: a lookup performed from the parser has a null semantics context, which skips the check-state suppression and reduces member lookup to direct members only.                                                                                      | internal-source-fact  | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes) | The null-semantics state exists only mid-parse; any `.slang` source observes the checked state instead, so the reduced result is never user-visible.                                                                                                 |
| C75, C86: `FileDecl` never passes the category filter, and `ExtensionExternVarModifier` / `ExternModifier` members of an `extension` are rejected before any mask bit is consulted.                                                                            | internal-source-fact  | [#edge-cases-and-failure-modes](../../../../design/name-resolution/lookup.md#edge-cases-and-failure-modes) | Slang has no syntax that names a file as an identifier, and both modifiers are attached by core-module and link-time machinery rather than by user source, so neither rejection has a reachable input.                                               |
| C83, C99: the walk skips a null container sentinel and a re-visited file scope, and appends items without comparing them, so the same decl reached two ways appears twice with different breadcrumb chains.                                                    | implementation-detail | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                     | Both are properties of the intermediate result rather than of the resolution: re-searching or duplicating an item yields the same decl, and the duplicate is consumed by the caller's narrowing before anything user-visible happens.                |
| C87: transparent-member recursion is skipped when the mask includes `Attribute`, and when `IgnoreTransparentMembers` is set for the base clause of a declaration that itself carries a transparent modifier.                                                   | internal-source-fact  | [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members)                   | Both short-circuits are cycle breakers on lookups the checker issues internally, and the doc states no user-written declaration carries the transparent modifier, so neither condition is selectable from source. Recorded as a doc gap below.       |
| C90, C95, C96: the facet walk skips ill-formed facets and, under `IgnoreBaseInterfaces`, interface facets; the scope walk forces `NoDeref` inside an `extension`; an `interface` scope is rewritten through its this-type decl with the breadcrumb suppressed. | internal-source-fact  | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                               | Each is a guard or rewrite selected by an option or a malformed facet that no Slang construct requests; the doc names no surface that sets `IgnoreBaseInterfaces` or writes an `extension` on a pointer-like type.                                   |
| C92: the pack-element type arms (`EachType`, `FirstPackElementType`, `LastPackElementType`, `PackBranchType`) canonicalize the type before entering the facet walk.                                                                                            | out-of-bundle         | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                               | These arms are reached only from variadic-generic expansion, whose surface and semantics belong to the generics/variadics design docs; this bundle's claims are about the lookup step itself.                                                        |
| C93: `AndType` reaching the member-lookup type dispatch is an internal error, because constraint flattening is supposed to have run first.                                                                                                                     | internal-source-fact  | [#member-lookup](../../../../design/name-resolution/lookup.md#member-lookup)                               | The claim is an invariant assertion for compiler developers; by construction no user source reaches it, and a test that did would be asserting a crash rather than a behavior.                                                                       |
| C101: lookup itself never diagnoses; every diagnostic named on the page is raised by a caller.                                                                                                                                                                 | internal-source-fact  | [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)                     | The split between the lookup step and its caller is not observable from source — the diagnostic is emitted either way. The diagnostics themselves are covered by C64, C66 and C67.                                                                   |

## Doc gaps observed

| Anchor                                                                                   | Kind            | Gap                                                                                                                                                                                                                                                                                                                                                                                                          | Suggested addition                                                                                                                                                                                                                      |
| ---------------------------------------------------------------------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#unqualified-lookup](../../../../design/name-resolution/lookup.md#unqualified-lookup)   | missing-surface | Step 5 says a `FunctionDeclBase` consults "`isEffectivelyStatic`, `[mutating]`, and `[ref]`", but writing `[ref]` on a method is rejected with `unknown attribute 'ref'` (E31000), so a reader cannot construct the third case. The `[mutating]` and static cases are both writable as stated.                                                                                                               | Name the declaration form that actually carries the `[ref]` qualifier (a `ref` property accessor, or wherever it is spelled), with a two-line example, or say that `[ref]` is compiler-internal and not writable as a method attribute. |
| [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members) | ambiguous-claim | The section says the one context that sets `IgnoreTransparentMembers` is "the check of a declaration's base clause when the declaration itself carries `TransparentModifier`", but the same section says every transparent member comes from a `cbuffer` / `tbuffer` / interface-block declaration, and those have no base clause. A reader cannot tell whether the option ever fires for user-written code. | State whether a buffer-block declaration can carry a base clause; if it cannot, say explicitly that this option is unreachable from user source and exists for compiler-internal lookups only.                                          |
| [#transparent-members](../../../../design/name-resolution/lookup.md#transparent-members) | missing-example | The section lists "the GLSL interface-block forms" as a third producer of transparent members alongside `cbuffer` and `tbuffer`, but gives no example of such a block and does not say which language mode or command-line option makes one writable, so that third producer cannot be tested.                                                                                                               | Add a two-line GLSL interface-block example together with the compilation mode it requires (e.g. the `-lang glsl` / `-allow-glsl` invocation), so all three producers of a transparent member have a reproducible surface.              |

## Sibling-bundle overlap

`docs/generated/tests/design/name-resolution/scopes/` owns the
**boundary** questions (from where is a name visible?); this bundle
owns the **algorithm** questions (once lookup starts here, what does
the name bind to?). Claims deliberately left to that bundle, and the
angle taken here instead:

- Block-local name not visible outside its block — scopes owns it.
  Here the block-local rule is probed from the other three sides
  instead: a hidden local falls through to an outer same-name decl
  ([`forward-use-with-outer-resolves-to-outer.slang`](forward-use-with-outer-resolves-to-outer.slang)),
  the rule does not apply to a non-variable decl
  ([`block-local-struct-visible-before-decl.slang`](block-local-struct-visible-before-decl.slang)),
  and a `catch` clause's error variable is un-hidden before its handler
  is checked
  ([`catch-clause-variable-visible-in-handler.slang`](catch-clause-variable-visible-in-handler.slang)).
- Forward reference inside a block rejected — scopes owns the flat
  single-block case; here the negative is angled at a _nested_ inner
  block ([`nested-block-forward-ref-rejected.slang`](nested-block-forward-ref-rejected.slang)).
- Generic parameter visible in the body / not visible in a sibling
  decl — scopes owns both visibility halves; here the claim is the
  shadowing consequence
  ([`generic-param-shadows-outer-typedef.slang`](generic-param-shadows-outer-typedef.slang)).
- Re-opened namespace reachable through a qualified name — scopes
  owns the qualified case; here both namespace tests exercise the
  _unqualified_ `using namespace` path that runs through the sibling
  loop of the scope walk.
- Extension method callable via the receiver — scopes owns the basic
  case; here the claim is the overload-set merge between a direct
  member and a same-named extension member.
- Builtin reachable from the implicit core-module import — scopes owns
  it entirely.
