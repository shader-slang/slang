---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ast-reference/base.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 5
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/base.md

## Summary

Nothing was escalated. This pass re-decided the five gaps the ledger
reports as REOPENED (recorded fixed, still reported by a regenerated
bundle); all five are `fixed` again. The previous fixes were already
present in the text, so each section was re-verified against the current
tree and strengthened where it was thin or had gone stale: the
`KnownBuiltinDeclName` row was wrong (it said thirteen entries; the enum
now has seventeen), `QualType` had gained flags the doc did not name, and
the `DeclBase` example did not say when no group is produced. The
sixth gap (362e71f6dc64, `DeclRefBase` heading) stays `deferred` in the
ledger and was not in this queue.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f2ca0e0557c1 | fixed | `source/slang/slang-ast-base.h:767` (`parentDecl`), `:771`+`:777` (`nameAndLoc`, `getNameLoc`), `:772`+`:800-801` (`inferredCapabilityRequirements`, `capabilityRequirementProvenance`), `:780`+`:793-797` (`checkState`, `setCheckState` release-asserts it only advances). Re-checked: all four bullets already name their consequence and cross-link `pipeline/03-semantic-check.md`; no source change in this header since the prior intake. | — no text change needed beyond re-verification; existing four bullets and the 03-semantic-check cross-link retained |
| dd9a7b37b697 | fixed | `source/slang/slang-ast-base.h:755-760` (`DeclBase`); `source/slang/slang-ast-decl.h:19` (`DeclGroup : public DeclBase`); `source/slang/slang-parser.cpp:2919-2985` (`DeclGroupBuilder`: group created only when a second name appears, shared modifiers marked, `getResult()` returns the plain `decl` otherwise) and `:3815-3841` (`ParseDeclaratorDecl`, "`int a = 1, b = a < 2;`"). Worked example `int a = 1, b = 2; static const int P = 3, Q = 4;` already in the section. | added that a single-name declaration is returned as a plain `Decl` and only a multi-name declarator list yields the group |
| 4311bafc8a6a | fixed | `source/slang/slang-ast-support-types.h:628-662` — `QualType` holds `type`, `isLeftValue`, `hasReadOnlyOnTarget`, `isWriteOnly`, and the new `isReadOnlyMemoryLocation` (never set with `isLeftValue`, per its comment); `source/slang/slang-ast-base.h:819` (`Expr::type`); `source/slang/slang-diagnostics.lua:1382-1384` (E30011 "left of '=' is not an l-value"). | extended the `Expr` `type: QualType` bullet with E30011 and the other three flags, and the `QualType` support-types row to list all four flags |
| 08d16522b6fe | fixed | `source/slang/slang-ast-base.h:714-718` (`keywordName`, `getKeywordNameAndLoc`); `source/slang/slang-check-modifier.cpp:497,531,549,567,584` pass `attr->keywordName` as `.attrName`; `source/slang/slang-diagnostics.lua:2793-2794` ("unknown attribute '~attrName:Name'"). The "attribute 'X' is not valid here" wording in the gap was not located in the watched paths and was not documented. | added that the attribute checker passes `keywordName` as the diagnostic's `attrName` |
| 74b52a8ce3a2 | fixed | `source/slang/slang-ast-support-types.h:226-244` (enum now has 17 entries incl. `CallShader`, `ReportHit = 16`) and `:247-269` (`isDifferentiableInterfaceBuiltin`); behaviour gates: `source/slang/slang-check-expr.cpp:4517-4524` (`OperatorAddressOf`), `:8386-8391` (`GetAttributeAtVertex`), `source/slang/slang-lower-to-ir.cpp:1525-1534` (decoration on the IR inst), `source/slang/slang-ir-ray-tracing-legalize.cpp:420,437`, `source/slang/slang-ir-autodiff.cpp:274-300`; tagging at `source/slang/core.meta.slang:5072`, `source/slang/hlsl.meta.slang:21063,21396`. | corrected the entry count to seventeen, added `CallShader`/`ReportHit`, and named concrete behaviour gates beyond the differentiable-interface family |
