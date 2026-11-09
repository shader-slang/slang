---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: d3dc6d6755c9a9d2b2dff12422b5e7bdc0cd26f90429fc52ddd7d6d51ab372b0
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Lookup

This document specifies Slang's name-lookup algorithm: the entry
points exposed by `slang-lookup.h`, the request and result types, the
per-step pipeline that walks scopes, inheritance facets, and
transparent members, and the shadowing rules that decide which decls
are visible at a given source location. The intended reader is a
developer modifying lookup behavior, a contributor adding a new
lookup-aware feature (a new modifier, a new declaration kind, a new
inheritance rule), or someone chasing an ambiguous-reference
diagnostic.

Visibility filtering on lookup results is described in
[visibility.md](visibility.md). Overload ranking that consumes a
multi-item `LookupResult` is described in
[overload-resolution.md](overload-resolution.md). The shape of the
scope chain that lookup walks is described in
[scopes.md](scopes.md).

## Source

The four entry points, the two `AddToLookupResult` overloads, and the
exported mask test `DeclPassesLookupMask` are declared in
[slang-lookup.h](../../../../source/slang/slang-lookup.h) and
implemented in
[slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp). The
data structures (`LookupMask`, `LookupOptions`, `LookupRequest`,
`LookupResult`, `LookupResultItem`, `LookupResultItem_Breadcrumb`)
live in
[slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h).
The per-decl `hiddenFromLookup` flag and the
`_prevInContainerWithSameName` link are on `Decl` in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h); the
member-list accessors lookup calls are on `ContainerDecl` in
[slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) /
[slang-ast-decl.cpp](../../../../source/slang/slang-ast-decl.cpp); the
transparent-member and lookup-suppressing modifiers are in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).
The linearized inheritance information that member lookup iterates is
computed in
[slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp).

Diagnostics raised on lookup results are declared in
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua).

Four further pieces of machinery round out the source inventory and
are cited below: the
`Facet` / `FacetList` / `InheritanceInfo` declarations in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h)
(lines 508-775); the post-lookup narrowing, filtering, and
breadcrumb-expansion steps in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
(`ConstructLookupResultExpr`, `resolveOverloadedLookup`,
`filterLookupResultByCheckedOptional`, `diagnoseAmbiguousReference`);
the `CompareLookupResultItems` comparator in
[slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp);
and the two parser helpers that drive lookup before semantic checking
begins, in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp).

## Concepts

- **Entry points.** Four functions declared in
  [slang-lookup.h](../../../../source/slang/slang-lookup.h):
  - `lookUp` (lines 22-30) — unqualified name lookup starting from a
    `Scope*`. Note that it does *not* take a `LookupOptions`; it takes
    two `bool` parameters, `considerAllLocalNamesInScope` and
    `ignoreTransparentMembers`, and folds them into the corresponding
    `LookupOptions` bits itself
    ([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp)
    lines 1034-1054).
  - `lookUpMember` (lines 33-40) — qualified lookup of `name` against
    a `Type*`. This is the entry point that accepts a full
    `LookupOptions`.
  - `lookUpDirectAndTransparentMembers` (lines 43-50) — direct lookup
    in one `ContainerDecl`, with transparent-member injection but
    without inheritance or extension walks. It has no `LookupOptions`
    parameter, so it initially supplies `LookupOptions::None` (the
    request initializer may still add `Completion` when `name` is the
    completion-request token)
    ([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp)
    lines 307-328).
  - `refineLookup` (line 18) — re-filter an existing `LookupResult`
    against a different `LookupMask`. The first three drive lookup;
    this one is a post-filter.

  `AddToLookupResult` (lines 65-66) is also exported, in an
  item-at-a-time and a merge-a-whole-result overload, because callers
  that build results outside `slang-lookup.cpp` (the visibility and
  optional-constraint filters) need to accumulate items the same way.
  `DeclPassesLookupMask` (line 14) is exported as well, so that code
  outside the lookup file can apply the same kind-and-`extern` test
  to a decl; the checker's transparent-member redeclaration check
  (see "Transparent members") is one such caller.
- `LookupRequest`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1585-1605) — the parameter bundle threaded through the lookup
  implementation: `semantics`, `scope`, `endScope`, `declToExclude`,
  `mask`, `options`, plus the two predicates `isCompletionRequest()`
  and `shouldConsiderAllLocalNames()`. Built by `initLookupRequest`
  ([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
  284-304), which also auto-sets the `Completion` option when the
  name being looked up matches the session's completion token. Two
  fields deserve a note:
  - `endScope` is never assigned by any caller at `source_commit`, so
    the scope walk always runs until the parent chain reaches null.
  - `declToExclude` is threaded from
    `SemanticsContext::getDeclToExcludeFromLookup`
    ([slang-check-impl.h](../../../../source/slang/slang-check-impl.h)
    lines 1703-1715) and exists so that a declaration being checked
    cannot find itself — the header's example is `typedef Foo Foo;`.
  - `semantics` may be null. Lookup performed from the parser has no
    `SemanticsVisitor` yet, and that changes behavior in two places
    (see step 3 of "Unqualified lookup" and "Member lookup" below).
- `LookupMask`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1330-1338) — a `uint8_t` bitset selecting which categories
  of decl pass the filter. The bits are:
  - `type = 0x1` — `AggTypeDecl` / `SimpleTypeDecl`.
  - `Function = 0x2` — `FunctionDeclBase` subclasses.
  - `Value = 0x4` — everything that is neither a type, a function, an
    attribute, a syntax decl, nor a semantic decl (variables,
    parameters, fields, ...); it is the fall-through case.
  - `Attribute = 0x8` — `AttributeDecl`, the declarations
    introduced by `attribute_syntax [name(...)]` in
    [core.meta.slang](../../../../source/slang/core.meta.slang) (line
    4708 declares `[numthreads]`). Source reaches them only by writing
    `[name(...)]` on a declaration
    ([slang-ast-decl.h line
    1229](../../../../source/slang/slang-ast-decl.h)).
  - `SyntaxDecl = 0x10` — keyword-introducing `SyntaxDecl`. The parser
    asks for this bit on its own when reading the modifier list of a
    parameter declaration, so that only a keyword decl can begin a
    modifier there
    ([slang-parser.cpp lines 5328 and
    8037](../../../../source/slang/slang-parser.cpp)).
  - `Semantic = 0x20` — `SemanticDecl`, the
    `semantic sv_position { ... }` declarations in
    [core.meta.slang](../../../../source/slang/core.meta.slang) (line
    5120) that record which types and stages a `: SV_*` annotation on a
    shader parameter is valid for
    ([slang-ast-decl.h line
    783](../../../../source/slang/slang-ast-decl.h)).
  - `Default = type | Function | Value | SyntaxDecl` — the mask the
    parser and most checker entry points use.

  `Attribute` and `Semantic` are the two bits *outside* `Default`, so
  only a caller that names them can reach those decls: an ordinary
  identifier spelled like an attribute neither hides the attribute nor
  is hidden by it. `type`, `Function`, and `Value` are already inside
  `Default`, so a use site that needs exactly one of those categories
  usually narrows the result after the fact through `refineLookup`
  rather than at the lookup call — the operand of `fwd_diff(...)` /
  `bwd_diff(...)` is resolved that way with `LookupMask::Function`
  ([slang-check-expr.cpp line
  6262](../../../../source/slang/slang-check-expr.cpp)).

  Classification happens in `DeclPassesLookupMask`
  ([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
  40-91). The mask test is not the first thing that function does: the
  `extern`-related rejections at lines 42-52 run before any bit is
  consulted, and `FileDecl` is hard-coded never to pass (lines 78-82).
- `LookupOptions`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1342-1357) — a `uint8_t` bitset of behavior flags. Besides
  `None = 0` there are six:
  - `IgnoreBaseInterfaces` (1 << 0) — skip inherited interface
    members.
  - `Completion` (1 << 1) — return every applicable decl in a
    container rather than only same-named ones, and do not stop the
    outward scope walk on the first hit; used by the language server.
  - `NoDeref` (1 << 2) — do not auto-dereference pointer-like types.
  - `ConsiderAllLocalNamesInScope` (1 << 3) — bypass the
    `hiddenFromLookup` / check-state shadowing test so that lookup
    can succeed while scopes are still under construction during
    parsing.
  - `IgnoreInheritance` (1 << 4) — skip non-`Self` inherited facets
    other than `extension`s on the same type. Interface facets are not
    governed by this option; they are controlled by
    `IgnoreBaseInterfaces`.
  - `IgnoreTransparentMembers` (1 << 5) — skip transparent-member
    injection.
- `LookupResultItem`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1448-1526) — one found decl plus an optional breadcrumb
  chain (`DeclRef<Decl> declRef`,
  `RefPtr<LookupResultItem_Breadcrumb> breadcrumbs`). It exposes
  `Breadcrumb` as a nested typedef for the free-standing
  `LookupResultItem_Breadcrumb` class.
- `LookupResultItem_Breadcrumb`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1364-1446) — a navigation step recorded during lookup. Its
  `Kind` enum (lines 1367-1396) has five values:
  - `Member` — lookup saw a transparent in-scope decl and looked
    through it, so the final expression needs `obj.field`.
  - `Deref` — lookup auto-dereferenced a pointer-like type, so the
    final expression needs `(*obj)`.
  - `SuperType` — lookup walked from a sub-type to a super-type via
    a `subtypeWitness`, so the final expression must reflect that
    super-typing.
  - `ThisExpr` — lookup found a member of an enclosing type or
    extension while an implicit `this` value is usable, so the final
    expression needs an implicit `this.`.
  - `ThisType` — lookup found a member of an enclosing type or
    extension where no `this` value is usable (a static context, or
    a reference from a nested type), so the final expression needs
    the `This` type as its base.

  Breadcrumb instances chain via
  `RefPtr<LookupResultItem_Breadcrumb> next`. A `ThisExpr` breadcrumb
  additionally carries the `Scope*` at which lookup began
  (`thisExprScope`, read through `getThisExprScope()`); the
  constructor asserts that this scope is non-null exactly when the
  kind is `ThisExpr` (line 1426). Lookup records no mutability for
  `this`: that is computed when the breadcrumb is expanded (see
  "Breadcrumbs").
- `LookupResult`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 1532-1579) — single-or-multi container for found items. A
  result is *valid* when `item.declRef.getDecl()` is non-null, and
  *overloaded* when `items.getCount() > 1`. The invariant documented
  at lines 1538-1542 is that when `items` is in use it holds *all* the
  items and `item` duplicates one of them (in practice the first);
  `items` is left entirely empty in the single-result case so that no
  heap allocation happens. `begin()`/`end()` hide the distinction from
  callers.
- `Facet` / `FacetList` / `InheritanceInfo`
  ([slang-check-impl.h](../../../../source/slang/slang-check-impl.h)
  lines 508-775) — the linearized inheritance information member
  lookup iterates. A facet is a `(kind, directness, origin,
  subtypeWitness, declRefForMemberLookup)` record; `origin` is the
  type and/or `DeclRef` whose members the facet contributes. `kind`
  is `Type` or `Extension`; `directness` is `Self` (0), `Direct` (1),
  or a larger indirection count. The list is singly linked through
  `FacetImpl::next` and is deduplicated by origin at construction
  time — `originsMatch` / `FacetList::containsMatchFor`
  ([slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp)
  lines 1969-1983 and 2060-2068) — so a base interface reached through
  two inheritance paths yields exactly one facet.

## Algorithm

### Unqualified lookup

```mermaid
flowchart TB
  start["lookUp(name, startScope, mask, considerAllLocalNames, declToExclude, ignoreTransparent)"]
  init["initLookupRequest -> LookupRequest"]
  scopeWalk["_lookUpInScopes: scope -> sibling chain -> parent"]
  perScope["per scope: dispatch on containerDecl kind"]
  typeBranch["AggTypeDeclBase: _lookUpMembersInType(This)"]
  defBranch["other: _lookUpDirectAndTransparentMembers"]
  facets["facet walk: self, bases, extensions"]
  trans["transparent-member injection"]
  filter["DeclPassesLookupMask"]
  result["LookupResult: empty / single / overloaded"]
  start --> init --> scopeWalk --> perScope
  perScope --> typeBranch --> facets --> trans
  perScope --> defBranch --> trans
  trans --> filter --> result
```

`lookUp`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
1034-1054) converts its two `bool` parameters into `LookupOptions`,
builds a `LookupRequest`, and calls `_lookUpInScopes`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
815-1032). The implementation does the following, in order:

1. **Iterate over scopes.** The outer loop walks
   `request.scope` to `request.endScope` via the `parent` chain
   ([slang-lookup.cpp line
   831](../../../../source/slang/slang-lookup.cpp)). Because no caller
   sets `endScope`, the walk continues until the parent chain reaches
   null, unless the early-stop rule below applies.
2. **Iterate over sibling scopes.** At each scope, the inner loop
   walks `nextSibling` from the current link so that sibling
   `NamespaceDecl`s, `using`-injected namespaces, and imported-module
   scopes are consulted at the same level
   ([slang-lookup.cpp line
   836](../../../../source/slang/slang-lookup.cpp)). See
   [scopes.md#sibling-scopes](scopes.md#sibling-scopes) for how those
   siblings get linked.
3. **Skip dummy and re-visited file scopes.** A null
   `containerDecl` is a sentinel that links siblings; it is
   skipped (lines 846-847). The first `FileDecl` encountered is
   remembered, and a later re-encounter of that *same* `FileDecl` is
   skipped so that a file whose scope appears twice on the chain is
   not searched twice
   ([slang-lookup.cpp lines
   849-858](../../../../source/slang/slang-lookup.cpp)).
4. **Dispatch on container kind.** Each `containerDecl` is first
   turned into a `DeclRef` by `createDefaultSubstitutionsIfNeeded`
   (lines 866-870). If the result is an `AggTypeDeclBase` — for
   example lookup happening *inside* a `struct`, `class`, `interface`,
   `enum`, or `extension` — the request is rewritten to perform member lookup
   against the corresponding `Type*`, with a `this` breadcrumb (of the
   kind tracked in step 5) that records the implicit `this`/`This` of
   the enclosing decl
   ([slang-lookup.cpp lines
   881-960](../../../../source/slang/slang-lookup.cpp)). Otherwise the
   request falls through to
   `_lookUpDirectAndTransparentMembers`
   ([slang-lookup.cpp lines
   961-975](../../../../source/slang/slang-lookup.cpp)).
5. **Track the kind of `this` breadcrumb.** `_lookUpInScopes` keeps two
   local variables, `thisBreadcrumbKind` and `thisBreadcrumbScope`,
   that start as `Breadcrumb::Kind::ThisExpr` and `request.scope`
   (lines 821-822). Every `this` breadcrumb created in step 4 — and the
   one created for an `InterfaceDefaultImplDecl` — takes its kind from
   the first variable and its `thisExprScope` from the second (lines
   890-892 and 988-990). After each scope link has been processed,
   `_doesScopeBlockUseOfEnclosingThisExpr`
   ([slang-lookup.cpp lines
   785-813](../../../../source/slang/slang-lookup.cpp)) is asked about
   that link's container decl (call at lines 1008-1013). A true answer
   switches the state to `Breadcrumb::Kind::ThisType` with a null
   scope, and nothing switches it back, so every farther-out type scope
   (including later sibling links) yields a `ThisType` breadcrumb. The
   helper strips any `GenericDecl` wrapper to the declaration it wraps,
   then answers:
   - false for a `ConstructorDecl`, because a constructor body has its
     own `this` even though `isEffectivelyStatic` treats constructors
     as static for calls;
   - true for an `AggTypeDeclBase` (a nested type or extension), so
     lookup outside it cannot use an instance of the enclosing type;
   - `isEffectivelyStatic(decl)` for a `CallableDecl` or
     `PropertyDecl`;
   - false for everything else, including ordinary block scopes.

   The helper's comment states that a false result leaves the existing
   state unchanged and does not establish that a `this` value is
   available. `slang-lookup.cpp` records no `this` mutability; see
   "Breadcrumbs".
6. **Short-circuit on a non-overloadable hit.** After visiting one
   scope and its siblings, if the result is valid and is neither
   overloaded, nor overloadable per `_isDeclOverloadable`
   ([slang-lookup.cpp lines
   765-783](../../../../source/slang/slang-lookup.cpp)), nor a
   completion request, lookup stops walking outwards
   ([slang-lookup.cpp lines
   1015-1029](../../../../source/slang/slang-lookup.cpp)). Callables
   (and generics wrapping them) are overloadable, so they continue to
   accumulate candidates from outer scopes; types and variables do
   not.
7. **Result.** The accumulated `LookupResult` is returned. Lookup
   itself never diagnoses; every diagnostic named on this page is
   raised by a caller.

`_lookUpDirectAndTransparentMembers`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
188-282) does the per-container work for the non-type branch:

- In completion mode it iterates *every* direct member via
  `getDirectMemberDecls` (lines 197-212); otherwise it iterates only
  members whose name matches the `name` argument, using
  `ContainerDecl::getDirectMemberDeclsOfName` (declared at
  [slang-ast-decl.h line
  191](../../../../source/slang/slang-ast-decl.h), defined at
  [slang-ast-decl.cpp line
  371](../../../../source/slang/slang-ast-decl.cpp)), which walks the
  same-name list threaded through
  `Decl::_prevInContainerWithSameName`
  ([slang-ast-base.h](../../../../source/slang/slang-ast-base.h) line
  790).
- Each candidate is filtered through `_isUncheckedLocalVar` to
  enforce block-local shadowing (see "Shadowing rules" below). In the
  named-lookup branch that test additionally requires
  `request.semantics` to be non-null (line 227), so a parse-time
  lookup never suppresses a local on check-state grounds.
- In the named-lookup branch each candidate is compared against
  `request.declToExclude` (the completion branch has no such test),
  then filtered through `DeclPassesLookupMask`. That filter also drops
  decls carrying `ExtensionExternVarModifier` and rejects
  `ExternModifier`-tagged members of `extension`s unconditionally
  ([slang-lookup.cpp lines
  42-52](../../../../source/slang/slang-lookup.cpp)).
- After direct members, the function walks
  `ContainerDecl::getTransparentDirectMemberDecls` and recurses into
  each transparent value via `_lookUpMembersInValue`, recording a
  `Breadcrumb::Kind::Member` step. Transparent-member injection is
  skipped when the mask includes `Attribute` or when
  `IgnoreTransparentMembers` is set
  ([slang-lookup.cpp lines
  245-281](../../../../source/slang/slang-lookup.cpp)).

### Member lookup

`lookUpMember(astBuilder, semantics, name, type, sourceScope, mask,
options)`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
1056-1068) is the entry point for `obj.name`. It calls
`_lookUpMembersInType` (lines 721-736), which only null-checks the
type and forwards to `_lookUpMembersInSuperTypeImpl` (lines 577-708).
That function is where the dispatch on type shape happens:

- **`DeclRefType`.** Lookup delegates to
  `_lookUpMembersInSuperTypeDeclImpl` (lines 512-575), described
  below.
- **`EachType` / `FirstPackElementType` / `LastPackElementType` /
  `PackBranchType`.** Lookup canonicalizes the type and enters the
  facet walk for the canonicalized form
  ([slang-lookup.cpp lines
  624-685](../../../../source/slang/slang-lookup.cpp)). All four of
  these arms return early when `request.semantics` is null, because
  computing `InheritanceInfo` requires the shared semantics context.
- **`ModifiedType`.** Modifiers are transparent to lookup; the facet
  walk runs on the modified type directly
  ([slang-lookup.cpp lines
  640-653](../../../../source/slang/slang-lookup.cpp)). Exactly three
  spellings produce one: `unorm`, `snorm`, and `no_diff`, the only
  modifiers `checkTypeModifier` turns into a modifier `Val`
  ([slang-check-expr.cpp lines
  9914-9931](../../../../source/slang/slang-check-expr.cpp)). The
  parser moves such a modifier off the declaration and onto its type
  expression (`_moveTypeModifiersToTypeExpr`,
  [slang-parser.cpp line
  3448](../../../../source/slang/slang-parser.cpp)), so a member
  access on a value declared `no_diff S` reaches this arm. The
  compiler also introduces `no_diff` on its own while building
  derivative function types (`getBackwardDiffFuncType`,
  [slang-check-expr.cpp lines
  5941-6070](../../../../source/slang/slang-check-expr.cpp)).
- **`ExtractExistentialType`.** The implicit `ThisType` decl-ref of
  the underlying interface is the target of lookup, so that
  associated types in a found member's signature resolve against a
  comparable substitution
  ([slang-lookup.cpp lines
  686-701](../../../../source/slang/slang-lookup.cpp)). The source
  shape that produces one is a variable, parameter, or field declared
  with an interface type: `maybeOpenExistential`
  ([slang-check-expr.cpp lines
  241-261](../../../../source/slang/slang-check-expr.cpp)) rewrites a
  member-access base whose type is a `DeclRefType` of an
  `InterfaceDecl` into an `ExtractExistentialValueExpr` typed
  `ExtractExistentialType` (lines 198-207). It looks through a
  `ModifiedType` first (`unwrapModifiedType`, line 249), so an
  interface-typed value wrapped in `no_diff` is opened too. It runs on
  the base of every member access (line 9378, in
  `maybeInsertImplicitOpForMemberBase`) and static member access (line
  9105, in `visitStaticMemberExpr`).

  ```
  interface ICounter { int val(); }
  struct S : ICounter { int val() { return 5; } }
  ICounter c = S();   // `c` has existential type
  int n = c.val();    // member lookup takes this arm
  ```
- **`AndType`.** Unexpected at lookup time;
  `visitGenericTypeConstraintDecl` is supposed to have flattened it
  earlier, so the arm is a `SLANG_UNEXPECTED`.
- Anything else — including `ErrorType` — falls off the end of the
  chain and contributes no items.

**Lookup in a decl.** `_lookUpMembersInSuperTypeDeclImpl` (lines
512-575) handles three cases in order. First, the name `This` in
anything other than an `InterfaceDecl` resolves to the decl-ref
itself (lines 521-527). Second, if `request.semantics` is null — the
parse-time case — it does a direct-members-only lookup on an
`AggTypeDeclBase` and returns, without consulting bases or
extensions (lines 533-548). Otherwise it drives the decl to
`DeclCheckState::ReadyForLookup` via `ensureDecl` (line 550), asks
`SharedSemanticsContext::getInheritanceInfo` for the linearized facet
list — keyed on the `ExtensionDecl` decl-ref for an extension, and on
the canonical self type otherwise (lines 555-565) — and hands off to
the facet walk.

**Facet walk.** `_lookupMembersInSuperTypeFacets`
([slang-lookup.cpp lines
392-510](../../../../source/slang/slang-lookup.cpp)) iterates
`inheritanceInfo.facets`. For each facet it:

- Skips facets with no `ContainerDecl` decl-ref, and facets missing
  either a type or a `subtypeWitness` (lines 405-415).
- Skips interface facets when `IgnoreBaseInterfaces` is set (lines
  420-424).
- For non-interface containers only (the check is an `else if` after
  the interface test, so interface facets are governed solely by
  `IgnoreBaseInterfaces`), skips non-`Self` facets when
  `IgnoreInheritance` is set, with a special case that keeps an
  `extension` whose target type equals the self type (lines 427-445).
- Skips facets whose `subtypeWitness` is a `DeclaredSubtypeWitness`
  over an inheritance decl carrying `IgnoreForLookupModifier` — the
  synthetic tag-type inheritance on `enum`s
  ([slang-lookup.cpp lines
  458-463](../../../../source/slang/slang-lookup.cpp)).
- Treats an inherited `This` specially: when the facet's container is
  a `ThisTypeDecl` and the name is `This`, an inherited candidate is
  suppressed entirely if the self type is an interface (an
  interface's own `This` must not be made ambiguous by its bases),
  and otherwise the facet's decl-ref *is* the result
  ([slang-lookup.cpp lines
  472-488](../../../../source/slang/slang-lookup.cpp)).
- For a non-`Self` facet of `Facet::Kind::Type`, prepends a
  `Breadcrumb::Kind::SuperType` step carrying the `subtypeWitness`
  (lines 492-499).
- Calls `_lookUpDirectAndTransparentMembers` on the facet's
  container, using `facet->declRefForMemberLookup` as the parent
  decl-ref (lines 501-508).

**How interface requirements arrive.** An interface facet does not
inject its requirements as a separate step; the injection is encoded
in the parent decl-ref. `FacetImpl::init`
([slang-lookup.cpp lines
350-374](../../../../source/slang/slang-lookup.cpp)) calls
`_maybeSpecializeSuperTypeDeclRef` (lines 330-348) for every
non-`Self` facet, which replaces an interface's plain decl-ref with a
`LookupDeclRef` built from the interface's `ThisTypeDecl` and the
facet's `subtypeWitness`. Members found through that facet therefore
come back already specialized to the concrete conforming type. See
[../ast-reference/values.md#declref-family-and-the-four-shapes-a-decl-ref-can-take](../ast-reference/values.md#declref-family-and-the-four-shapes-a-decl-ref-can-take)
for the four decl-ref shapes and
[../ast-reference/values.md#witness-family](../ast-reference/values.md#witness-family)
for the witness values involved.

**Pointer auto-dereference.** Before the per-type dispatch above,
`_lookUpMembersInSuperTypeImpl`
([slang-lookup.cpp lines
586-608](../../../../source/slang/slang-lookup.cpp)) calls
`getPointedToTypeIfCanImplicitDeref(superType)`. If the type is
pointer-like and `NoDeref` is not set, a `Deref` breadcrumb is
prepended and lookup recurses on the pointee; a valid result there
short-circuits the rest of the dispatch. The `_lookUpInScopes`
dispatcher forces `NoDeref` when the enclosing scope is an
`ExtensionDecl`
([slang-lookup.cpp lines
943-952](../../../../source/slang/slang-lookup.cpp)) so that the
extension's `This` refers to the extension target itself, not the
pointed-to type.

**`ThisType` for interfaces.** When the enclosing scope is an
`InterfaceDecl`, `_lookUpInScopes` rewrites the lookup to go through
the interface's `ThisTypeDecl` (the abstract self-type of the
interface). The `this` breadcrumb is suppressed in that case because
the rewritten decl-ref already encodes the navigation
([slang-lookup.cpp lines
919-935](../../../../source/slang/slang-lookup.cpp)); the rewrite is
skipped when the name being looked up is the `ThisTypeDecl`'s own
name. `InterfaceDefaultImplDecl` triggers a separate path that looks
up in the explicit `This` parameter instead of the interface itself
([slang-lookup.cpp lines
977-1006](../../../../source/slang/slang-lookup.cpp)).

### Transparent members

A `TransparentModifier`
([slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
line 129) on a member of a `ContainerDecl` causes its own members to
be searched whenever the parent is searched. The canonical example
documented in
[slang-ast-support-types.h lines
1460-1475](../../../../source/slang/slang-ast-support-types.h) is an
HLSL `cbuffer C { float4 f; }`: the compiler lowers this to

```
struct Anon0 { float4 f; };
__transparent ConstantBuffer<Anon0> anon1;
```

so that an unqualified reference to `f` resolves through
`anon1.f` via a chain of two breadcrumbs:

1. `Member` — `f` lives one struct member deep through `anon1`; the
   transparent member is recorded first.
2. `Deref` — `ConstantBuffer<Anon0>` is pointer-like, so lookup
   dereferences it.

`__transparent` is not a source-writable modifier — no keyword in the
parser's tables introduces one. The single site that creates a
`TransparentModifier` is `ParseBufferBlockDecl`
([slang-parser.cpp line
4409](../../../../source/slang/slang-parser.cpp), in the function
starting at line 4259), which backs the `cbuffer` and `tbuffer`
declaration keywords
([slang-parser.cpp lines
11114-11115](../../../../source/slang/slang-parser.cpp)) and the GLSL
interface-block forms that route to the same helper from
`ParseDeclWithModifiers` (lines 6213-6260). Every transparent member
in a user program therefore comes from one of those buffer-block
declarations.

A block produces a transparent member only when it has **no instance
name**: the final `else` branch that creates the modifier
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
4387-4411) is reached only after the named form (lines 4368-4375) and
the GLSL bindless-array form (lines 4376-4386) have been ruled out, so
`cbuffer C { float a; } c;` is *not* transparent while
`cbuffer C { float a; };` is. How the parser selects its source
dialect for the GLSL interface-block forms is a parsing concern and
is not covered here.

`ContainerDecl::getTransparentDirectMemberDecls`
([slang-ast-decl.h line
212](../../../../source/slang/slang-ast-decl.h), defined at
[slang-ast-decl.cpp line
422](../../../../source/slang/slang-ast-decl.cpp)) returns the cached
list of direct members carrying `TransparentModifier`; the cache is
populated when the container's lookup accelerators are rebuilt
([slang-ast-decl.cpp lines
300-303](../../../../source/slang/slang-ast-decl.cpp)). The lookup
side
([slang-lookup.cpp lines
257-281](../../../../source/slang/slang-lookup.cpp)) walks that list,
prepends a `Member` breadcrumb, and recurses via
`_lookUpMembersInValue`. The recursion is short-circuited when:

- the request's `mask` includes `Attribute` (transparent-member
  recursion is forbidden for attributes to avoid infinite recursion
  on transparent types that themselves contain attribute members),
  or
- the request's `options` include `IgnoreTransparentMembers`. The one
  context that sets that option is the check of an inheritance
  clause whose parent declaration itself carries
  `TransparentModifier`, where looking back through the transparent
  member would be circular
  (`SemanticsDeclBasesVisitor::visitInheritanceDecl` calling
  `excludeTransparentMembersFromLookup`,
  [slang-check-decl.cpp lines
  5149-5158](../../../../source/slang/slang-check-decl.cpp)). That
  combination is **not reachable from user source**: the sole
  producer of `TransparentModifier`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4409) attaches it to the synthesized *variable* declaration of an
  unnamed buffer block, and a `VarDecl` has no base clause for
  `visitInheritanceDecl` to be checking. Read the option as a guard
  for compiler-internal lookups rather than as something a program can
  provoke; it reaches lookup through `visitVarExpr`
  ([slang-check-expr.cpp lines
  5573-5581](../../../../source/slang/slang-check-expr.cpp)).

Lookup in a container finds the members of its transparent variables,
but those members are not direct members of the container, so
`checkForRedeclaration` does not compare them with the container's own
declarations. `SemanticsVisitor::checkForRedeclarationOfTransparentMembers`
([slang-check-decl.cpp lines
15035-15070](../../../../source/slang/slang-check-decl.cpp)) closes
that gap: it treats each member of a transparent variable's aggregate
type as declared at the variable, and compares each declaration with
the nearest earlier one of its name through `RedeclarationSeverity::Warning`
(`compareWithNearestVisibleDecl`, lines 15006-15021). It runs once per
container after the container's own members reach `DefinitionChecked`
([slang-check-decl.cpp lines
19565-19572](../../../../source/slang/slang-check-decl.cpp)) and at the
end of `visitBlockStmt`
([slang-check-stmt.cpp line
154](../../../../source/slang/slang-check-stmt.cpp)). Its source comment
gives:

```
static const int x = 1;
cbuffer CB { int x; }
```

as the motivating case, and states that a conflict is a warning
because earlier releases did not compare these pairs.

### Breadcrumbs

Each lookup result item carries a singly-linked list of
`LookupResultItem_Breadcrumb` nodes. The checker walks the list in
`SemanticsVisitor::ConstructLookupResultExpr`
([slang-check-expr.cpp lines
934-1105](../../../../source/slang/slang-check-expr.cpp)) to
synthesize the canonical AST expression. For the `cbuffer` example
above, an unqualified `f` becomes the equivalent of `(*anon1).f`:

- `Member` -> `Deref` -> (decl `f`).

For an unqualified `g` defined on `Self` inside an instance method,
the breadcrumb is just `ThisExpr`, marking that the rewritten
expression needs an implicit `this.g`. The `ThisExpr` arm of
`ConstructLookupResultExpr` (lines 1039-1080) creates a `ThisExpr`
whose `scope` is the breadcrumb's `thisExprScope` and checks it
through `checkThisExprWithoutCapturing`
([slang-check-expr.cpp lines
9588-9708](../../../../source/slang/slang-check-expr.cpp)), the same
routine that checks an explicit `this`. That routine starts from the
scope, defaults the expression to immutable, and searches upward for
the enclosing declaration: a `ConstructorDecl` makes `this` an
l-value (lines 9600-9603); a `FunctionDeclBase` takes the type and
writability from its effective `this` parameter information
(`isThisExprWritable`); an `AggTypeDeclBase` or
`InterfaceDefaultImplDecl` yields the `This` type; and running out of
scopes diagnoses `ThisExpressionOutsideOfTypeDecl`. The `[mutating]`
and `[__ref]` modifiers therefore matter to the checker's `this`
computation, not to lookup. In a static context or from a nested
type, the breadcrumb is `ThisType` instead, and the `ThisType` arm
(lines 1008-1037) builds a `SharedTypeExpr` over `calcThisType` of the
breadcrumb's decl-ref, so the expression starts from `This` rather
than from a value.

For lookup through an interface base via `subtypeWitness`, the
breadcrumb is `SuperType` and carries the witness as its `val`. That
field is also what
`SemanticsVisitor::filterLookupResultByCheckedOptional`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
lines 1394-1417) inspects: it walks each item's breadcrumb chain and
drops the item when any `SubtypeWitness` on it comes from an
`optional` constraint that the surrounding code has not yet checked.

`CreateLookupResultItem`
([slang-lookup.cpp lines
145-164](../../../../source/slang/slang-lookup.cpp)) reverses the
on-stack `BreadcrumbInfo` chain (declared at lines 29-36, with the
`thisExprScope` field) when constructing the heap-allocated linked
list, so the final order matches the navigation order from the user's
source expression to the found decl.

## Shadowing rules

### Block-local shadowing

Inside a `BlockStmt`, decls are temporarily hidden by setting
`Decl::hiddenFromLookup`
([slang-ast-base.h](../../../../source/slang/slang-ast-base.h) line
803). `SemanticsStmtVisitor::visitBlockStmt`
([slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp)
lines 95-155) sets the flag on every `DeclStmt` in the block's
`SeqStmt` body, and on each member of a `DeclGroup`, before checking
that body (lines 121-146). `visitDeclStmt` (lines 58-72) hands each
declaration (each member of a group, in order) to
`checkDeclAtDeclarationPoint` (lines 74-90), which clears the flag
(line 87) as the checker walks past it. The flag is only ever written
by these two statement-visitor paths plus `visitCatchStmt` (line 707),
which clears it on a `catch` clause's error variable (line 712).

Lookup honors the flag via `_isUncheckedLocalVar`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) lines
174-180), which treats a decl as not-yet-declared when it is
`Unchecked`, currently being checked, *or* `hiddenFromLookup`, and
when `isLocalVar` holds for it.

`LookupOptions::ConsiderAllLocalNamesInScope` lets a caller bypass
this check. The one caller that sets it is `tryLookUpSyntaxDecl`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
1165-1191), which also passes a null `SemanticsVisitor`: during
parsing, decls inserted so far have no meaningful check state, so the
flag lets the parser see them anyway when deciding whether an
identifier names a `SyntaxDecl`.

### Container-level overload accumulation

Decls with the same name inside one `ContainerDecl` do not shadow
each other; they accumulate into a `LookupResult` and become an
overload set. The chain is implemented by the
`Decl::_prevInContainerWithSameName` field
([slang-ast-base.h line
790](../../../../source/slang/slang-ast-base.h)). It is populated
lazily when a container's lookup accelerators are rebuilt —
`ContainerDeclDirectMemberDecls::_ensureLookupAcceleratorsAreValid`
([slang-ast-decl.cpp](../../../../source/slang/slang-ast-decl.cpp)
lines 259-342, writing the field at line 330) — and not by
`addDirectMemberDecl` (line 408), which only appends to the member
list. The same pass deliberately skips a `GenericDecl`'s `inner`
member (lines 311-319) so that a generic and its inner decl do not
both answer to the generic's name. `getDirectMemberDeclsOfName`
([slang-ast-decl.cpp line
371](../../../../source/slang/slang-ast-decl.cpp)) exposes the chain
as an iterable list, and lookup consumes it at
[slang-lookup.cpp line
222](../../../../source/slang/slang-lookup.cpp).

#### Deduplication

`AddToLookupResult`
([slang-lookup.cpp lines
94-112](../../../../source/slang/slang-lookup.cpp)) appends each
incoming `LookupResultItem` to the result without comparing it
against previously-collected items, so lookup performs no
deduplication of its own. Duplicates are prevented — or not — at two
other layers:

- **Within member lookup**, the facet list is already deduplicated by
  origin (`originsMatch`,
  [slang-check-inheritance.cpp lines
  1969-1983](../../../../source/slang/slang-check-inheritance.cpp)),
  so a base type reached through several inheritance paths
  contributes one facet and its members appear once.
- **Across lookup paths**, nothing dedupes. The same `DeclRef` reached
  both directly from an enclosing scope and through a transparent
  member appears twice, with different breadcrumb chains. Separately,
  lookup can return competing items for *different* declarations, such
  as an interface requirement alongside the concrete member that
  satisfies it. Narrowing is the caller's job: `SemanticsVisitor::_resolveOverloadedExprImpl`
  ([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
  lines 1564-1627) first calls `refineLookup` (line 1579) to drop items
  that do not match the contextually expected `LookupMask`, then hands
  the result to `resolveOverloadedLookup` (line 1582); how competing
  items are ranked is described in
  [overload-resolution.md#tie-breaking-comparator](overload-resolution.md#tie-breaking-comparator).

Keeping every breadcrumb path visible through lookup is what lets
these later phases produce accurate ambiguity diagnostics.

### Module and namespace

Multiple `namespace Foo {}` declarations under the same parent
container parse into the same `NamespaceDecl` — `parseNamespaceDecl`
([slang-parser.cpp line
4689](../../../../source/slang/slang-parser.cpp)) reuses the first one
it finds in that parent. Each `{ ... }` body, and each qualified
declaration that names the namespace (such as `A` in `namespace A::B`),
gets its own `Scope`; the parser records all of them in
`NamespaceDecl::lexicalScopes`
([slang-ast-decl.h line
859](../../../../source/slang/slang-ast-decl.h);
[slang-parser.cpp lines
155-180](../../../../source/slang/slang-parser.cpp)). Namespaces that
cannot be merged that way (same-named namespaces in different
`FileDecl`s of one module, same-named namespaces in different modules,
and the targets of a `using` declaration) are instead attached to the
scope chain as siblings, and the inner loop of step 2 above is what
makes them reachable. The wiring happens in
`SemanticsDeclScopeWiringVisitor`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
declared at line 591, with `visitImplementingDecl`, `visitUsingDecl`,
and `visitNamespaceDecl` at lines 18909-19071), a dedicated check
phase dispatched at `DeclCheckState::ScopesWired` (line 19519), which
sits between `ModifiersChecked` and `SignatureChecked`
([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
lines 495-509). `visitNamespaceDecl` attaches each same-named
namespace visible from the parent chain to *every* lexical scope of the
namespace being wired (lines 19024-19066) and then drives the
namespace's own `using` members to `ScopesWired` (lines 19067-19070);
`visitUsingDecl` adds the sibling scopes of each namespace-like entity
its argument names (lines 18960-19022).

The order in which `SemanticsDeclVisitorBase::checkModule` (line 5463)
reaches that state has three stages
([slang-check-decl.cpp lines
5600-5678](../../../../source/slang/slang-check-decl.cpp)):

1. Every `NamespaceDecl` in the module is driven to `ScopesWired`
   (lines 5635-5640). The source comment explains that the next stage
   resolves extension target types by unqualified lookup, which reads
   only the already-wired sibling chain, and that this stage covers
   only namespaces, not every declaration.
2. Each extension is driven through the `states` array up to and
   including `ReadyForLookup` (lines 5644-5657).
3. For each state in `ScopesWired`, `ReadyForReference`,
   `ReadyForLookup`, `ReadyForConformances`, `DefinitionChecked`,
   `CapabilityChecked` (declared at lines 5600-5607), `ensureAllDeclsRec`
   advances every declaration rooted in the module to that state before
   the next state begins (lines 5660-5678).

For an ordinary declaration, stage 3 means the whole module reaches
`ScopesWired` before any declaration advances past it, so lookups
performed while resolving that declaration's header already see the
complete sibling chain. That ordering is intentional and user-visible:
a `using` declaration may appear *after* the declaration whose header
depends on it, and the header still resolves.

```
namespace NS { struct Widget { int v; }; }
Widget makeW(int n) { Widget w; w.v = n; return w; }  // resolves
using namespace NS;
```

Extensions are the exception to "every declaration is wired first":
stage 2 runs them to `ReadyForLookup` after only the namespace
pre-pass. The source comment at lines 5609-5617 states that driving an
extension does not advance any namespace; it does not say whether a
module-scope `using` (one that is not a member of a namespace) has
been wired by then, and this document does not claim either way.

`addSiblingScopeForContainerDecl` (declared at
[slang-ast-decl.h line
1249](../../../../source/slang/slang-ast-decl.h), called at
[slang-check-decl.cpp line
19060](../../../../source/slang/slang-check-decl.cpp) for namespaces
and line 18990 for `using`) is the constructor for those links.
`importModuleIntoScope` (line 18663, called from
`SemanticsDeclHeaderVisitor::visitImportDecl` at line 18750, so not
from the `ScopesWired` visitor) filters what an `import` re-exports
through `isOwnModuleOrIncludedFileScope` (call at line 18700; defined
in [slang-check-expr.cpp line
326](../../../../source/slang/slang-check-expr.cpp)).
[scopes.md#sibling-scopes](scopes.md#sibling-scopes) owns the details;
[visibility.md](visibility.md) owns the reachability rules the filter
upholds.

### Interface requirements vs default implementations

A user-provided extension member can shadow an interface default
implementation. The relevant special case is
`InterfaceDefaultImplDecl`
([slang-ast-decl.h line
1002](../../../../source/slang/slang-ast-decl.h)): when lookup is
performed from inside one, `_lookUpInScopes` looks up in the explicit
`This` type parameter, then advances the scope cursor past every
scope up to and including the enclosing `InterfaceDecl` and breaks
out of the sibling loop
([slang-lookup.cpp lines
977-1006](../../../../source/slang/slang-lookup.cpp)), so the
interface's own requirement declarations are never consulted and a
witness override on a conforming type wins over the default.

### Keyword vs identifier

Keywords are `SyntaxDecl`s registered in the core module; they
share the identifier namespace with user decls, so shadowing is
decided by ordinary lookup rather than by a separate keyword table
(see
[../ast-reference/declarations.md#syntaxdecl-and-the-syntax-as-declaration-model](../ast-reference/declarations.md#syntaxdecl-and-the-syntax-as-declaration-model)).
Two parser helpers use lookup in opposite directions:

- `tryLookUpSyntaxDecl`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
  1165-1191) asks with `considerAllLocalNamesInScope = true` and
  rejects the result unless the single found decl is a `SyntaxDecl`.
  A local of the same name therefore wins, because it is found first
  and is not a `SyntaxDecl`.
- `isKeywordAvailable`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
  10060-10071) treats a *contextual* keyword as available only when
  plain lookup of that identifier finds nothing at all, so any user
  declaration of the name disables the keyword. It returns true
  without looking anything up when the parser has no
  `semanticsVisitor` or no `currentLookupScope`.

### Generic parameters

Generic parameters live in the `GenericDecl`'s own scope and are
seen as direct members of that scope. A reference to `T` from
inside the generic's inner decl resolves through the inner scope's
parent (the `GenericDecl` scope), shadowing any same-named decl in
the enclosing scope. A reference to `T` from a sibling of the outer
decl never finds the generic parameter because that scope chain
does not pass through the `GenericDecl`.

## Edge cases and failure modes

- **`LookupResult` with multiple items matching the mask.** Returned
  as overloaded; ranking is deferred to
  [overload-resolution.md](overload-resolution.md). When such a result
  is later re-filtered against a narrower `LookupMask` and only one
  item matches, `refineLookup`
  ([slang-lookup.cpp lines
  127-143](../../../../source/slang/slang-lookup.cpp)) drops every
  item failing `DeclPassesLookupMask` silently and returns the single
  survivor — no diagnostic is raised for the filtered-out candidates.
  `refineLookup` also returns its input unchanged when the input is
  invalid or not overloaded. Its caller in the checker is
  `SemanticsVisitor::_resolveOverloadedExprImpl`
  ([slang-check-expr.cpp line
  1579](../../../../source/slang/slang-check-expr.cpp)), which is
  handed the narrower mask by the use site; the operand of
  `fwd_diff(...)` / `bwd_diff(...)` asks for `LookupMask::Function`
  that way
  ([slang-check-expr.cpp line
  6262](../../../../source/slang/slang-check-expr.cpp)), so a
  same-named non-function candidate is dropped there without a
  diagnostic.
- **Ambiguous reference at use site.** When narrowing leaves more than
  one candidate and the context needs exactly one, the checker calls
  `diagnoseAmbiguousReference`
  ([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
  line 1613, inside `_resolveOverloadedExprImpl`, when a diagnostic
  sink was supplied), whose two-argument form (lines 1534-1549) emits
  `Diagnostics::AmbiguousReference` (`ambiguous-reference`, code 39999,
  [slang-diagnostics.lua lines
  4411-4417](../../../../source/slang/slang-diagnostics.lua)) followed
  by one `OverloadCandidate` note per surviving item; the
  single-argument `diagnoseAmbiguousReference` wrapper (lines
  1551-1562) is what sets the expression's type to the error type. A
  `NamespaceDecl` first item is exempted in
  `maybeDiagnoseAmbiguousReference` (lines 1520-1532) because an
  overloaded namespace reference is legitimate.
- **Forward reference inside a `BlockStmt`.** Using `b` before its
  `DeclStmt` reaches the lookup with `hiddenFromLookup = true`;
  `_isUncheckedLocalVar` skips the decl, so lookup returns either
  the decl from an outer scope or empty. The checker's
  `Diagnostics::UndefinedIdentifier` takes over from there, with a
  "did you mean" suggestion produced by the separate scope walk in
  `findClosestInScopeName` (see
  [scopes.md#scope-walking-order-during-lookup](scopes.md#scope-walking-order-during-lookup)).
- **`FileDecl` returns no hits.** `DeclPassesLookupMask` rejects
  `FileDecl` unconditionally
  ([slang-lookup.cpp lines
  78-82](../../../../source/slang/slang-lookup.cpp)) — its members are
  found through its sibling-linked module scope, not by directly
  looking up "the file" as a name.
- **`ExtensionExternVarModifier` and `ExternModifier` in
  extensions.** Both are filtered out at the very start of
  `DeclPassesLookupMask`
  ([slang-lookup.cpp lines
  42-52](../../../../source/slang/slang-lookup.cpp)), before any mask
  bit is consulted, so an `extern` member of an `extension` is never
  even considered a candidate.
- **Member lookup on `ErrorType`.** `ErrorType` is a direct `Type`
  subclass, not a `DeclRefType`, so it matches none of the arms in
  `_lookUpMembersInSuperTypeImpl` and lookup silently returns empty
  rather than cascading a second error.
- **Member lookup with no semantics context.** A parse-time
  `lookUpMember` on a `DeclRefType` reaches
  `_lookUpMembersInSuperTypeDeclImpl` with `request.semantics == null`
  and only sees direct members
  ([slang-lookup.cpp lines
  533-548](../../../../source/slang/slang-lookup.cpp)); inherited and
  extension members are simply absent. The pack-type arms return
  nothing at all in that state.
- **Transparent-member recursion when looking up an attribute.**
  Forbidden by the early return at
  [slang-lookup.cpp lines
  245-249](../../../../source/slang/slang-lookup.cpp): otherwise a
  transparent member that is itself an attribute target could
  trigger infinite recursion.
- **`AndType` reaching the type dispatch.** Signals a constraint-
  flattening bug; `_lookUpMembersInSuperTypeImpl` triggers
  `SLANG_UNEXPECTED("AndType should have been flattened ...")`
  ([slang-lookup.cpp lines
  702-707](../../../../source/slang/slang-lookup.cpp)).
- **`IgnoreForLookupModifier` on a base.** The synthetic tag-type
  inheritance on `enum`s carries this modifier
  ([slang-check-decl.cpp line
  13330](../../../../source/slang/slang-check-decl.cpp)) and is
  therefore filtered twice: once when the linearized facet list is
  built
  ([slang-check-inheritance.cpp line
  903](../../../../source/slang/slang-check-inheritance.cpp)) and
  again defensively in the facet walk
  ([slang-lookup.cpp lines
  458-463](../../../../source/slang/slang-lookup.cpp)), so the
  underlying integer type is not surfaced as a base when looking up
  enum members. See
  [visibility.md#interaction-with-ignoreforlookupmodifier](visibility.md#interaction-with-ignoreforlookupmodifier).
- **Inherited `This` in a derived interface.** Without the
  suppression at
  [slang-lookup.cpp lines
  472-481](../../../../source/slang/slang-lookup.cpp), an interface
  that inherits from another interface would find both `This`
  declarations and plain `This` would become ambiguous.
- **Unchecked `optional` constraint on the path to a member.** The
  member is found by lookup but removed afterwards by
  `filterLookupResultByCheckedOptional`; if that empties an
  otherwise-valid result, the diagnosing wrapper reports
  `Diagnostics::RequiredConstraintIsNotChecked`
  ([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
  lines 1420-1437), in both modes; in language-server mode the
  unfiltered result is subsequently returned instead. The constraint is written
  with `optional` after `where`
  ([slang-parser.cpp line
  2016](../../../../source/slang/slang-parser.cpp)), and the witness
  it produces counts as checked only inside an enclosing `if` whose
  predicate is an `is` test naming the same sub- and super-type
  (`isWitnessUncheckedOptional`,
  [slang-check-expr.cpp lines
  1348-1391](../../../../source/slang/slang-check-expr.cpp)):

  ```
  interface IPrintable { int code(); }

  int f<T>(T v) where optional T : IPrintable
  { return v.code(); }                                 // rejected

  int g<T>(T v) where optional T : IPrintable
  { if (T is IPrintable) return v.code(); return -1; } // accepted
  ```

## See also

- [scopes.md](scopes.md) — the scope chain that lookup walks.
- [visibility.md](visibility.md) — visibility filtering applied to
  lookup results; see
  [visibility.md#where-visibility-is-filtered](visibility.md#where-visibility-is-filtered).
- [overload-resolution.md](overload-resolution.md) — ranking of
  the overloaded `LookupResult` lookup may return; see
  [overload-resolution.md#probe-phase-where-candidates-come-from](overload-resolution.md#probe-phase-where-candidates-come-from).
- [../ast-reference/base.md](../ast-reference/base.md) — reference
  for `Decl`, `Scope`, and the support types `DeclRef`,
  `LookupResult`.
- [../ast-reference/values.md](../ast-reference/values.md) —
  reference for `LookupDeclRef` and the witness-related `Val`
  family.
- [../ast-reference/declarations.md](../ast-reference/declarations.md)
  — reference for `ContainerDecl`, `NamespaceDecl`, `FileDecl`,
  `InterfaceDefaultImplDecl`, and `SyntaxDecl`.
- [../ast-reference/modifiers.md](../ast-reference/modifiers.md) —
  reference for `TransparentModifier`, `IgnoreForLookupModifier`,
  and the extension/extern modifiers.
- [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
  — pipeline-level overview of where lookup runs.
- [../glossary.md](../glossary.md) — entries for `lookup result`,
  `lookup mask`, `lookup options`, `lookup breadcrumb`,
  `transparent member`, `shadowing`.
