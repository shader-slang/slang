---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:05+00:00
target_doc: glossary.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 8033723409ecbf2551b9a4eb228a4e39356c3fa79164d7d057fb8526b4b0145a
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for glossary.md

## Summary
The glossary is mostly aligned with its prompt: entries are tagged, term definitions are source-supported, and the sampled links resolve. One completeness issue remains: the `## Cross-reference index` table omits two peer pipeline documents included by the manifest's watched-path glob.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show glossary.md` and reviewed the per-doc prompt, `_common.md`, resolved watched files, and dependencies `architecture/overview.md`, `pipeline/overview.md`, `cross-cutting/ir-instructions.md`, `syntax-reference/keywords-and-builtins.md`, `name-resolution/index.md`, and `ir-reference/index.md`.
- Checked front matter for required keys, source commit, watched-path digest, and warning string.
- Verified the required structure: `## Conventions`, `## Terms`, and `## Cross-reference index`.
- Checked that the required Slang-specific and general-theory floor terms are present and tagged with exactly one of `[Slang]` or `[General]`.
- Spot-checked more than 10 source-alignment claims for terms including `ASTBuilder`, `capability atom`, `DiagnosticSink`, `FIDDLE`, `IRInst`, `IROp`, `layout IR module`, `lookup result`, `mandatory optimization pass`, `target`, and `witness table`.
- Checked relative links and peer links used by the page; no unresolved target was found.
- Checked that the body has no source line-number citations requiring line-by-line verification.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Cross-reference index`, lines 757-792 | The prompt requires the cross-reference index to cover every peer doc listed in the manifest entry for `glossary.md`, but the table omits `pipeline/04b-pre-link-passes.md` and `pipeline/04c-layout-ir.md`. Both docs are included by the manifest's `docs/generated/design/pipeline/*.md` watched-path glob and were present in the resolved file list from `regenerate.py show glossary.md`. | `docs/generated/design/_meta/prompts/glossary.md:135` requires the index table to cover every peer doc listed in the manifest entry. `docs/generated/design/_meta/manifest.yaml:754` through `docs/generated/design/_meta/manifest.yaml:757` include the pipeline glob for `glossary.md`; the generated table lists `pipeline/04-ast-to-ir.md`, then jumps to `pipeline/05-ir-passes.md` and `pipeline/06-emit.md`. | Add rows for `pipeline/04b-pre-link-passes.md` and `pipeline/04c-layout-ir.md`, using the existing terms `mandatory optimization pass` and `layout IR module` respectively. |
