---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: architecture/module-map.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: f11187e79ffaa9e1c1966046a9bb76df4bb33cabc81ba4f496d6f224fcf0ca12
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for architecture/module-map.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked prompt-required section shape, source/core and compiler-core rows, AST/check/emit rows, prelude header list, standard module links, and cross-cutting links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | multiple tables | The page lists many `.cpp` implementation files even though the manifest watched set for this doc includes CMake files, `prelude/*.h`, and `slang-*.h` headers, not arbitrary `.cpp` files. | `docs/generated/design/_meta/manifest.yaml` watched paths for `architecture/module-map.md` exclude `source/slang/*.cpp`. | Either expand watched paths to include the cited `.cpp` files or rewrite rows to cite watched headers and CMake paths. |
| F-002 | minor | `## source/slang/` | The prompt names `slang-linkage*`, but the `Linkage` row uses `slang-linkable.h` / implementation files; source declares `Linkage` in `slang-session.h`. | `source/slang/slang-session.h` declares `class Linkage`. | Include `slang-session.h` / `.cpp` in the `Linkage` row or adjust the row label to linkable component types. |
