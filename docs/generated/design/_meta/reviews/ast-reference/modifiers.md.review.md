---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:20+00:00
target_doc: ast-reference/modifiers.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 09bdb006642550c81ee966bcb8ea28e65ed6abdbbb21b36d0fbd18f7f1a7472b
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 0
  minor: 2
  nit: 0
---

# Review report for ast-reference/modifiers.md

## Summary
The modifiers page satisfies the family coverage rule: every concrete `FIDDLE()` class in `slang-ast-modifier.h` appears in the Nodes tables, and abstract classes are not table rows. The remaining issues are small source-alignment errors in the `Key fields` column for two rows.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/modifiers.md` and used the listed prompt, dependency docs, and watched files at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`.
- Compared all 254 concrete `FIDDLE()` classes in `source/slang/slang-ast-modifier.h` against the `## Nodes` tables and verified that no `FIDDLE(abstract)` class appears as a row.
- Checked immediate parent names in the Nodes tables against the header declarations.
- Spot-checked source-backed claims for modifier-vs-attribute syntax, HLSL semantics, GLSL layout grouping, intrinsic modifiers, target intrinsic modifiers, capability attributes, memory qualifier aggregation, differentiability attributes, and matrix-layout modifiers.
- Resolved the relative links and anchors; the body has no source line-number citations.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Nodes`, `IntrinsicOpModifier` row | The `Key fields` cell lists `opcode: int`, `irOp: uint32_t`, but those are not the field names in the watched header. The class stores the parsed token in `opToken` and the IR opcode in `op`. | `source/slang/slang-ast-modifier.h:264-274` declares `Token opToken` and `FIDDLE() uint32_t op = 0` on `IntrinsicOpModifier`. | Change the key fields to `opToken: Token`, `op: uint32_t` or summarize only `op: uint32_t` if the token is not useful for readers. |
| F-002 | minor | `## Nodes`, `GLSLUnparsedLayoutModifier` row | The `Key fields` cell says `text Token`, but `GLSLUnparsedLayoutModifier` declares no additional fields beyond its `Modifier` base. | `source/slang/slang-ast-modifier.h:416-420` declares `class GLSLUnparsedLayoutModifier : public Modifier` with only `FIDDLE(...)` in the body. | Change the key fields cell to `(no additional state)` and keep the summary focused on its role as a raw layout qualifier placeholder. |
