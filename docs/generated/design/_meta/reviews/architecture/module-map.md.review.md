---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:11:45+00:00
target_doc: architecture/module-map.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 3e84425a669764ca340ed7a1190856897496ac2f667956b0a8228b11c7f4ab35
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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

# Review report for architecture/module-map.md

## Summary
The source-backed spot checks and relative links look good, but the page is only partially complete against its prompt contract. The remaining issue is structural: several `source/` subdirectories introduced by `overview.md` are collapsed into a two-column catch-all table instead of receiving their own level-2 logical-unit sections.

## Items checked
- Ran `regenerate.py show architecture/module-map.md` and reviewed the manifest prompt, watched files, and `depends_on` peer `architecture/overview.md`.
- Verified front matter, required link style, all 56 markdown links, and prompt-required table shape for the main `source/core`, `source/compiler-core`, `source/slang`, standard-module, and `prelude` sections.
- Spot-checked 18 symbol/file responsibility claims against commit `52339028a2aa703271533454c6b9528a534bac31`, including `Linkage`, `Session`, `Module`, `DiagnosticSink`, `IRBuilder`, `slang-emit*`, `slang-check*`, `slang-parser*`, and prelude headers.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Other source/ subdirectories`, lines 278-288 | The page collapses multiple overview logical-unit groups (`source/slang-llvm/`, `source/slang-glslang/`, `source/slang-dispatcher/`, `source/slang-rt/`, `source/slang-record-replay/`, `source/slang-wasm/`, and `source/slangc/`) into a single two-column table for subdirectory and role. The prompt requires each logical unit group from `overview.md` to have its own level-2 section followed by a table with `Logical unit`, `Files`, and `Responsibility` columns. | `docs/generated/design/_meta/prompts/architecture-module-map.md:23-31` specifies the per-group level-2 heading and required table columns, and `docs/generated/design/_meta/prompts/architecture-module-map.md:66-67` requires every logical unit group in `overview.md` to have its own level-2 section. `docs/generated/design/architecture/overview.md:104-125` introduces the affected downstream, runtime/bindings, and driver groups individually. | Split the catch-all section into one level-2 section per affected `source/` group and give each section the required `Logical unit`, `Files`, and `Responsibility` table, even if a small group only has one row. |
