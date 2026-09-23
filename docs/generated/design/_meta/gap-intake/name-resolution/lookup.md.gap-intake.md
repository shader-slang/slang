---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:47:44Z
target_doc: name-resolution/lookup.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 7
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for name-resolution/lookup.md

## Summary

Nothing was escalated and nothing was deferred: all seven gaps are
`missing-surface` or `missing-example` requests for the user-visible
entry point of a mechanism the page already described correctly, and
every one of them was confirmable in the watched paths. Seven gaps are
`fixed`. Two fixes also corrected a claim the confirmation pass showed
to be unsupported: the page said `IgnoreTransparentMembers` is "used,
for example, when looking up an unscoped-enum's underlying type",
whereas the only context that sets it is the base-clause check of a
declaration that itself carries `TransparentModifier`
(`source/slang/slang-check-decl.cpp:4798-4804`, reaching lookup via
`visitVarExpr` at `source/slang/slang-check-expr.cpp:5342`); and the
`__transparent` modifier turned out to have no source spelling at all
— the sole producer is the `cbuffer` / `tbuffer` / GLSL-interface-block
parse path. The mask-bit and `refineLookup` gaps share one underlying
observation (which construct asks for a category narrower than
`Default`) and were answered as one rule stated in `## Concepts` and
referenced from the `refineLookup` bullet.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                             | Fix summary                                                                                                                                                             |
| ------------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| b3a0611e138c | fixed  | `source/slang/slang-ast-decl.h:1168` (`AttributeDecl` is "used with `[name(...)]` syntax") and `source/slang/core.meta.slang:4599` (`attribute_syntax [numthreads(...)]`); `source/slang/slang-ast-decl.h:729` (`SemanticDecl` "validate `: SV_*` annotations") and `source/slang/core.meta.slang:5001` (`semantic sv_position`); `source/slang/slang-parser.cpp:5059,7655` pass `LookupMask::SyntaxDecl` alone for a parameter's modifier list; `source/slang/slang-check-expr.cpp:6012` narrows to `LookupMask::Function` for a `fwd_diff`/`bwd_diff` operand; bundle test `lookup-mask-attribute-vs-value-name.slang` | gave `Attribute`, `SyntaxDecl`, and `Semantic` their source-writable entry points and added a closing rule for `type` / `Function` / `Value` (shared with c2b0943711e4) |
| 07d3cdfe2c16 | fixed  | `source/slang/slang-parser.cpp:1947` parses `optional` after `where` into `OptionalConstraintModifier`; `source/slang/slang-check-expr.cpp:1311-1355` (`isWitnessUncheckedOptional`) keeps the witness only when an enclosing `IfStmt` predicate is an `IsTypeExpr` over the same sub/sup pair; bundle tests `optional-constraint-unchecked-member-rejected.slang` and `optional-constraint-member-visible-after-is-check.slang`                                                                                                                                                                                          | added the rejected/accepted `where optional T : I` contrast and the `is`-discharge rule to the edge-case bullet; refreshed the stale wrapper line range to 1383-1402    |
| c2b0943711e4 | fixed  | `source/slang/slang-check-expr.cpp:1527-1542` (`_resolveOverloadedExprImpl` calls `refineLookup` with the context mask); `source/slang/slang-check-expr.cpp:6012` (`_checkHigherOrderInvokeExpr` resolves the operand with `LookupMask::Function`); `source/slang/slang-parser.cpp:10865-10869` registers `fwd_diff` / `bwd_diff`                                                                                                                                                                                                                                                                                        | named the checker-side caller and the `fwd_diff(...)` / `bwd_diff(...)` operand as the narrow-mask input shape                                                          |
| 8e866ce37099 | fixed  | `source/slang/slang-check-expr.cpp:240` (`maybeOpenExistential` on a `DeclRefType` of an `InterfaceDecl`), `:197-206` (builds the `ExtractExistentialType` / `ExtractExistentialValueExpr`), `:8824` and `:8744` (run on every member-access and static-member base); bundle test `existential-member-lookup-through-thistype.slang`                                                                                                                                                                                                                                                                                     | added the interface-typed-local source shape and a four-line example to the `ExtractExistentialType` arm                                                                |
| bf138b97df57 | fixed  | `source/slang/slang-check-expr.cpp:9356-9375` (`checkTypeModifier` accepts exactly `unorm`, `snorm`, `no_diff`; `const` / `volatile` are diagnosed instead); `source/slang/slang-parser.cpp:3300` (`_moveTypeModifiersToTypeExpr` moves a `TypeModifier` onto the declaration's type expression); `source/slang/slang-check-expr.cpp:5711-5776` (compiler-introduced `no_diff`)                                                                                                                                                                                                                                          | named the three modifiers that produce a `ModifiedType` and how they reach a declaration's type                                                                         |
| 52a2aaeadd31 | fixed  | `source/slang/slang-ast-support-types.h:492-506` (`ScopesWired` between `ModifiersChecked` and `SignatureChecked`); `source/slang/slang-check-decl.cpp:5245-5252,5305-5321` (`ensureAllDeclsRec` runs once per state, in order, so the whole module is wired first); bundle test `using-wired-before-signature-check.slang`                                                                                                                                                                                                                                                                                              | added the state-loop citation, the "intentional" statement, and a three-line later-`using` example                                                                      |
| 2d86ccbc9c23 | fixed  | `source/slang/slang-parser.cpp:4159` is the only site in `source/` that creates a `TransparentModifier` (grep for `__transparent` finds comments only), reached from `cbuffer` / `tbuffer` (`:10727-10728`) and the GLSL interface-block paths (`:5868-5891`); `source/slang/slang-check-decl.cpp:4798-4804` and `source/slang/slang-check-expr.cpp:5342` are the only producers/consumers of `IgnoreTransparentMembers`                                                                                                                                                                                                  | stated that `__transparent` is not source-writable and listed the buffer-block producers; replaced the unsupported unscoped-enum example for `IgnoreTransparentMembers` |
