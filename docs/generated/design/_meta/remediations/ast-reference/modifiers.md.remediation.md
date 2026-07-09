---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:55Z
target_doc: ast-reference/modifiers.md
review_report: ../../reviews/ast-reference/modifiers.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/modifiers.md

## Summary
Three of the four findings target genuine `Key fields` errors and were fixed; one was rejected as bogus. F-001 is rejected: all four classes it names already listed their fields correctly in the target doc at the start of remediation. F-002, F-003, and F-004 each contained one or more accurate field corrections, which were applied. Several example claims inside F-002 and F-003 were themselves wrong (the rows they cite were already correct); only the genuinely-wrong rows were edited. No front-matter fields were touched; the source commit is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | At the start of remediation all four cited rows already named their fields, contradicting the claim of `(no additional state)`: `SynthesizedModifier` (`op: uint32_t`, `operands: List<Val*>`), `ExplicitlyDeclaredCapabilityModifier` (`declaredCapabilityRequirements: CapabilitySetVal*`), `ExtensionExternVarModifier` (`originalDecl: DeclRef<Decl>`), `HasInterfaceDefaultImplModifier` (`defaultImplDecl: Decl*`); these match `source/slang/slang-ast-modifier.h:183-184,199,234,792`. | — |
| F-002 | fixed | `VulkanHitObjectAttributesAttribute` row said `(no additional state)` but `source/slang/slang-ast-modifier.h:1365` declares `int location`. The finding's `WaveSizeAttribute`/`EntryPointAttribute` examples were already correct in the doc; only the genuinely-wrong row was changed. | `VulkanHitObjectAttributesAttribute`: `(no additional state)` -> `location: int`. |
| F-003 | fixed | `DifferentiableAttribute` had invented `mode` (no such field; `source/slang/slang-ast-modifier.h:1650` stores `m_associatedValMapping`), and `BackwardDifferentiableAttribute` said `(no additional state)` but `slang-ast-modifier.h:1876` declares `int maxOrder`. | `DifferentiableAttribute`: `mode` -> `m_associatedValMapping` map; `BackwardDifferentiableAttribute`: `(no additional state)` -> `maxOrder: int`. |
| F-004 | fixed | All three rows mismatched the header: `slang-ast-modifier.h:1700-1702` (`modulePath`, `functionName`), `:1739-1740` (`fwdDiffFuncDeclRef`, `bwdDiffFuncDeclRef`), `:1782` (`guid`). | `DllImportAttribute` -> `modulePath: String, functionName: String`; `AutoPyBindCudaAttribute` -> `fwdDiffFuncDeclRef: DeclRefExpr*, bwdDiffFuncDeclRef: DeclRefExpr*`; `ComInterfaceAttribute` -> `guid: String`. |
