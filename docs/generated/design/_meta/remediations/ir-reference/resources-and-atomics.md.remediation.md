---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:12:00Z
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary
Three findings were fixed and one was rejected as out of scope. The mandatory
`AST origin` column header was restored across all fourteen opcode subtables,
the one passage that described purely target-specific lowering was trimmed, and
`slang-lower-to-ir.cpp` visitor citations were added to a buffer row and the
shader-IO row. The hierarchy finding was rejected because the per-document
prompt explicitly requires the topical sub-group diagram the reviewer wants
removed. The document was edited, so `mark-fresh` is needed.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/_common.md:232-241` fixes the column name as `AST origin`; the page used `Produced by`. The cell *content* was left naming the concrete producing declaration/pass, which is the settled IR-reference convention and is what `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:51-52` asks for; `(synthesized)`/`—` catch-alls are deliberately not used. | Renamed the fifth column header in all 14 opcode subtables and the prose reference at line 73. |
| F-002 | rejected-out-of-scope | The recommendation contradicts the per-document contract: `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:82` requires "Hierarchy diagram covers the seven sub-groups above", i.e. the topical grouping. Per `_remediate.md:108-109` the contract wins. The page already states at lines 133-135 that only `AtomicOperation` and `BindingQuery` are Lua grouping parents, so no reader is misled. | — |
| F-003 | fixed | Only the `imageGatherOffset` callout described target-specific lowering with no IR-level purpose (SPIR-V `ConstOffset` vs `Offset` and the `ImageGatherExtended` capability), which `ir-reference-resources-and-atomics.md:75-77` forbids. The other cited passages are not lowering descriptions: lines 272-275 and 513-520 cite emit call sites as *evidence* for the absence of a scope operand and for positional memory-order layout, and lines 624-651 are a producer/consumer census recording verified latent source bugs. | Replaced the SPIR-V image-operand/capability sentence with a pointer to `../pipeline/06-emit.md`. |
| F-004 | fixed | The prompt's quality checklist (`ir-reference-resources-and-atomics.md:83-84`) requires a buffer row and a shader-IO row to cite a `slang-lower-to-ir.cpp` visitor; neither named one. Verified `visitInvokeExpr` at `source/slang/slang-lower-to-ir.cpp:7172` and `visitVarDecl` at `:11927` (which reaches `lowerGlobalVarDecl` at `:11743`, `lowerGlobalShaderParam`, then `createGlobalParam` at `:11588`). | `byteAddressBufferLoad` row now cites `visitInvokeExpr` line 7172; `global_param` row now cites `visitVarDecl` line 11927 and the call chain to `createGlobalParam`. |
