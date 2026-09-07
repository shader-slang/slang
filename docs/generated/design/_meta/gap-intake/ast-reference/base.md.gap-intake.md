---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:40:37Z
target_doc: ast-reference/base.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/base.md

## Summary

Nothing was escalated: every observation in this queue was confirmed in
the watched headers, and none of them contradicted the document. Five
gaps were fixed by editing four sections — the `Decl` and `Modifier`
field lists, the `Expr` `type: QualType` bullet, the `DeclBase` prose
(which gained a two-line Slang example), and the `KnownBuiltinDeclName`
row of `## Support types`. One gap was deferred: promoting `DeclRefBase`
to its own `### DeclRefBase (Val)` subsection would add an eleventh
heading to the ten the generation prompt enumerates for `## Roots`, so
the durable fix is a prompt amendment rather than a document edit.
Nothing was rejected.

The most useful confirmation of the cycle was for
`KnownBuiltinDeclName`, where the gap asked whether the enum is a
behaviour gate or only a speed-up. It is both: the core module attaches
the enumerator to a declaration with `[KnownBuiltin(n)]`
(`core.meta.slang:4955`, applied at `core.meta.slang:647` and
`hlsl.meta.slang:20842`), and the header's own comment on
`isDifferentiableInterfaceBuiltin` names it the authoritative definition
of the differentiable-interface family, whose conformance witness-table
entries the linker defers for programs that do not use auto-diff.

Operator follow-up: the `Decl::inferredCapabilityRequirements` and
`checkState` bullets now cross-link `pipeline/03-semantic-check.md` for
the diagnostic and the phase sequence, because the code that produces
both (`slang-check-decl.cpp`, `slang-check-shader.cpp`) is outside this
page's `watched_paths`. The document therefore states only what
`slang-ast-base.h` itself says about those two fields and points
elsewhere for the rest.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f2ca0e0557c1 | fixed | `source/slang/slang-ast-base.h:767` (`parentDecl`), `:771`+`:777` (`nameAndLoc` / `getNameLoc` returns `nameAndLoc.loc`), `:772`+`:800-801` (`inferredCapabilityRequirements` and the `capabilityRequirementProvenance` comment "Track the decl reference that caused the requirement of a capability atom"), `:780`+`:793-797` (`setCheckState` release-asserts the state only advances), `:798` (`isChildOf`). Observables verified by `docs/generated/tests/design/ast-reference/base/decl-name-and-parent.slang` and `decl-name-loc-in-redefinition.slang`; the capability note wording and the `DeclCheckState` sequence are described in `docs/generated/design/pipeline/03-semantic-check.md:387-405` and `:512`. | extended each of the four `Decl` field bullets with its observable consequence, and cross-linked `03-semantic-check.md` from the capability and check-state bullets |
| dd9a7b37b697 | fixed | `source/slang/slang-ast-base.h:755-760` (`DeclBase` is "an intermediate type to represent either a single declaration, or a group of declarations"); the grouped syntax and the shared-modifier consequence are the verified forms in `docs/generated/tests/design/ast-reference/base/declbase-group-multiple-names.slang` (`int a = 1, b = 2, c = 3;`) and `declbase-group-shares-modifiers.slang` (`static const int P = 3, Q = 4;` plus `struct V { int x, y; };`) | added a two-line `slang` example under the `DeclBase` paragraph plus a sentence that modifiers written once in front of the group reach every name it declares |
| 4311bafc8a6a | fixed | `source/slang/slang-ast-support-types.h:625-632` — `QualType` is `Type* type` together with `bool isLeftValue`; `source/slang/slang-ast-base.h:819` declares `Expr::type` as that `QualType`. The resulting diagnostic is the verified form in `docs/generated/tests/design/ast-reference/base/expr-qualtype-rvalue-not-assignable.slang` (`makeValue() = 5;` -> "left of '=' is not an l-value", E30011) | added the `isLeftValue` clause and the `f() = 5` example to the `type: QualType` bullet |
| 08d16522b6fe | fixed | `source/slang/slang-ast-base.h:714-718` — the comment names `keywordName` as "the keyword that was used to introduce ... this modifier", and `getKeywordNameAndLoc()` pairs it with the node's `loc`. The quoted-back spelling is the verified form in `docs/generated/tests/design/ast-reference/base/modifier-keyword-name-in-diagnostic.slang` (`unknown attribute 'NoSuchAttributeXyz'`) | added a clause to the `keywordName` bullet naming the attribute diagnostic that reproduces the recorded spelling |
| 74b52a8ce3a2 | fixed | `source/slang/slang-ast-support-types.h:226-242` (the enum) and `:247-257` (the comment declaring `isDifferentiableInterfaceBuiltin` "the authoritative definition of the differentiable interface family", consulted by the IR linker to defer conformance witness-table entries when auto-diff is unused); `source/slang/core.meta.slang:4955` declares `attribute_syntax [KnownBuiltin(name : int)]`, applied at `core.meta.slang:647` (`IDifferentiable`) and `hlsl.meta.slang:20842` (`DispatchMesh`) | rewrote the `KnownBuiltinDeclName` row to say how a declaration acquires the tag and that membership gates behavior, with the differentiable-interface family as the example |
| 362e71f6dc64 | deferred | Blocked by the generation contract: `docs/generated/design/_meta/prompts/ast-reference-base.md:32-46` enumerates exactly ten `## Roots` subsections and their order, and `DeclRefBase` is not among them. The document folds both non-listed abstract roots of `source/slang/slang-ast-base.h` into listed ones the same way — `SyntaxNodeBase` (`:695-699`) into `### SyntaxNode (SyntaxNodeBase)` and `DeclRefBase` (`:630-679`) into `### Val (NodeBase)` — so promoting only `DeclRefBase` would be inconsistent and would be dropped by the next regeneration. Follow-up: amend that prompt's root list (adding `DeclRefBase`, and probably `SyntaxNodeBase` for symmetry) and regenerate, after which decl-ref tests can re-anchor. | — |
