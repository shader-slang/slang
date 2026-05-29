---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:30:00+00:00
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary

One major finding addressed by adding representative per-family
summary tables under `## Instruction families`. Each table follows
the prompt's `(Opcode, struct_name, Operands, Notes)` shape, is
explicitly representative (not exhaustive), and ends with a
"see ir-reference/X.md for the full list" row, satisfying the
prompt's "summary-level: at most a few dozen entries per family"
constraint.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/cross-cutting-ir-instructions.md` step 4 requires "For each family produce a table" using a four-column shape; the original page only linked to family pages. The full opcode catalog correctly lives in `ir-reference/*`, but a representative summary still belongs here so the page satisfies its prompt and gives a reader enough context to confirm they are on the right family page. | Inserted summary tables for the eight families called out by the prompt (Type, Value, Memory, Control-flow, Function/module structure, Specialization/existentials, Decorations, Resource/shader-IO) immediately after the existing family-page bullets. Each table picks 4-7 representative opcodes and ends with a row cross-linking to the canonical `ir-reference/*.md` page for the full list. The Notes column is one short phrase per row, matching the prompt's example. |
