---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:09:00Z
target_doc: pipeline/03-semantic-check.md
review_report: ../../reviews/pipeline/03-semantic-check.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for pipeline/03-semantic-check.md

## Summary

Three of the four minor findings were verified and fixed with local edits: two
responsibility-table cells, the over-broad binding-modifier gate, and the
invented errored-declaration state. F-002, the section-scope violation in
`## Generic specialization and constraints`, is deferred: it is valid but needs
a 140-line condensation rather than a minimum-necessary edit. Nothing was
rejected or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-check-resolve-val.cpp:1-24` is titled "Logic for resolving/simplifying Types and DeclRefs" and implements `createCanonicalType` / `_resolveImplOverride`, not substitution validation; visibility filtering lives in `slang-check-expr.cpp` / `slang-check-overload.cpp`. | `## SemanticsVisitor` table: inheritance row now "Inheritance and extension lookup; facet computation"; resolve-val row now "Resolves and canonicalizes `Type`, `DeclRef`, and witness values". |
| F-002 | deferred | Valid: the prompt requires "overview only" (`docs/generated/design/_meta/prompts/pipeline-03-semantic-check.md:42-44`), and the section runs roughly 140 lines of solver, associated-type, inheritance-cache, and differentiability internals. Condensing it is a section-scale rewrite, not a minimum-necessary edit, and the reviewer's own no-issues notes confirm the content is source-accurate, so discarding it by hand risks losing verified material and conflicting with peer pages. Follow-up: regenerate this document (or amend the prompt clause if the detail is wanted) in a later cycle. | — |
| F-003 | fixed | `source/slang/slang-check-shader.cpp:2336-2345` applies `supportsVkBindingOnParameter` only in the `GLSLBindingAttribute` branch; `:2346-2366` diagnoses `PushConstantAttribute`, `HLSLRegisterSemantic`, and `HLSLPackOffsetSemantic` unconditionally. | `## Shader-specific checks`: gate restricted to `[[vk::binding(...)]]`, with a new closing clause stating the other three are diagnosed unconditionally. |
| F-004 | fixed | `source/slang/slang-ast-support-types.h:475-560` defines `DeclCheckState` with no errored member (it ends at `DefinitionChecked` / `CapabilityChecked`), and `source/slang/slang-check-decl.cpp:5298-5322` runs `ensureAllDeclsRec` over the state sequence for every declaration. | `## Failure modes` closing paragraph: rewritten to say `checkModule` drives declarations to `CapabilityChecked` and that recovery is diagnostics plus substituted error types/expressions. |
