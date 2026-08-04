# Prompt: name-resolution/lookup.md

See [_common.md](_common.md) for the universal rules and the
**Name-resolution family contract** that applies to this page.

## Target

Produce `docs/generated/design/name-resolution/lookup.md` — the page that
documents the unqualified-name and member-lookup algorithms, the
filtering masks and option flags, the breadcrumb mechanism, and the
shadowing rules.

This is the meatiest page in the subtree (size cap 64 KB).

Audience: a developer modifying lookup behavior; a developer chasing an
ambiguous-reference diagnostic; a contributor implementing a new
lookup-aware feature (new modifier, new declaration kind, new
inheritance rule).

## Family-specific guidance

The page uses the `## Algorithm` heading.

The `## Concepts` section must cover, at minimum:

- The four lookup entry points in
  [slang-lookup.h](../../../../source/slang/slang-lookup.h): `lookUp`,
  `lookUpMember`, `lookUpDirectAndTransparentMembers`, and
  `refineLookup` (or whatever the four actual entry points are at
  `source_commit`).
- `LookupRequest` — declared in
  [slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h).
- `LookupResult` — the multi-item container in
  `slang-ast-support-types.h`.
- `LookupResultItem` — the single result, including its `declRef`.
- `LookupResultItem::Breadcrumb` (or `LookupResultItem_Breadcrumb`) —
  the linked structure that records the navigation path
  (member-of, deref, witness-table lookup, ...) used to reconstruct
  the canonical expression.
- `LookupMask` — declare its named bits (`type`, `Function`, `Value`,
  `Attribute`, `SyntaxDecl`, `Semantic`, plus anything else present
  at `source_commit`) and what each is for.
- `LookupOptions` — declare its flags (`IgnoreBaseInterfaces`,
  `Completion`, `NoDeref`, `ConsiderAllLocalNamesInScope`,
  `IgnoreInheritance`, `IgnoreTransparentMembers`, plus anything else
  present at `source_commit`) with one-line use-case descriptions.

The `## Algorithm` section must contain three sub-sections:

### Unqualified lookup

Walk the algorithm step by step:
1. Start from the current scope.
2. Walk sibling scopes in order.
3. Walk to the parent scope and repeat.
4. For each visited scope, walk its container's direct member decls,
   inheritance chain, and any active extensions.
5. Inject transparent members (see below).
6. Apply the `LookupMask` filter.
7. Deduplicate by `DeclRef`.
8. Return a `LookupResult` that is empty / single / overloaded.

Cite the controlling function for each step (the name in `slang-lookup.cpp`).

Include a mermaid flowchart of the pipeline. Suggested shape:

```
identifier+startScope -> walkScopeChain -> directMembers -> inheritance -> extensions -> transparentMembers -> maskFilter -> dedupe -> LookupResult
```

### Member lookup

Cover `lookUpMember(Type*, Name*)`. Explain:
- How `DeclRefType` resolves to the underlying `Decl`'s container.
- How `ThisType` is handled.
- How interfaces inject their requirement set.
- The pointer auto-deref controlled by `LookupOptions::NoDeref`.
- Existential / `IExistentialType` handling (if present at
  `source_commit`).

### Transparent members

Explain the `TransparentModifier` (declared in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)) and
`ContainerDecl::getTransparentDirectMemberDecls`. Use the typical
example pattern — a field tagged transparent injects its own members
into the parent. Cite the call site in `slang-lookup.cpp` that drives
the injection. Mention `IgnoreTransparentMembers` as the opt-out path.

### Breadcrumbs

Explain how breadcrumbs reconstruct the canonical expression. For an
unqualified `f` that turns out to be a member of `this`, the breadcrumb
chain records `Member` + `Deref`; the checker uses this to materialize
`(*this).f` as the rewritten expression. List the
`Breadcrumb::Kind` (or equivalent) values present at `source_commit`.

## Shadowing rules

This is a required level-2 heading inside the page (sibling to the
`## Algorithm` sections above, after them, before
`## Edge cases and failure modes`).

Cover, in this order:

1. **Block-local shadowing.** The
   `Decl::hiddenFromLookup` flag (cite its declaration in
   [slang-ast-base.h](../../../../source/slang/slang-ast-base.h)) toggled
   on entry to a `BlockStmt` and cleared as the checker walks past
   each `DeclStmt`. The `ConsiderAllLocalNamesInScope` option overrides
   this for keyword-shadowing detection.
2. **Container-level overload accumulation.** Same-name decls inside
   one container are not shadowed; they form an overload set. Cite the
   `_prevInContainerWithSameName` field (or its actual name) and the
   `ContainerDecl::getDirectMemberDeclsOfName` accessor.
3. **Module / namespace shadowing.** Multiple `namespace Foo {}`
   declarations are collapsed via sibling scopes; `using`-decl
   injection follows the same rules as direct members.
4. **Interface / extension overlay.** A user-supplied extension member
   can shadow an interface default impl; cite the helper in
   `slang-lookup.cpp` that resolves this conflict.
5. **Keyword vs identifier.** Keywords are `SyntaxDecl` entries that
   share the identifier namespace; the `LookupMask::SyntaxDecl` bit
   separates them.
6. **Generic-parameter shadowing.** Generic parameters shadow
   enclosing-scope names of the same identifier inside the generic
   body.

The `## Edge cases and failure modes` section must cover:

- `LookupResult` with multiple items but only one matches the mask —
  the others are filtered silently.
- Ambiguous references with no preferred candidate; cite
  `Diagnostics::ambiguousReference` (or its actual identifier at
  `source_commit`) from
  [slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).
- Transparent-member chains creating multiple paths to the same decl;
  how `LookupResult` dedupes.
- Forward reference inside a `BlockStmt` (`hiddenFromLookup` is set
  -> diagnostic `useOfUndeclaredIdentifier`).
- Member lookup on `ErrorType` is silently empty.

## Forbidden content (in addition to the universal forbidden list and the family contract)

- Visibility filtering — that belongs in
  [visibility.md](visibility.md). Mention only that visibility is
  applied at or after lookup, with a forward reference.
- Overload ranking — that belongs in
  [overload-resolution.md](overload-resolution.md). Lookup may return
  an overloaded `LookupResult`; the ranking happens later.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every named `LookupMask`, `LookupOptions`, and `Breadcrumb::Kind`
      value is present in
      [slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
      at `source_commit`.
- [ ] Every algorithm step cites the controlling function in
      `slang-lookup.cpp`.
- [ ] Shadowing rules are grounded in concrete fields/functions; no
      ad-hoc claims.
