---
review_report: true
reviewer_model: gpt-6-astra
reviewed_at: 2026-11-03T08:20:38Z
target_doc: cross-cutting/core-module.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: d4e2be8995c8c95a45df9e148d2a3e7eca71adcea84c5108fd81038c3a296350
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
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
  major: 0
  minor: 2
  nit: 0
---

# Review report for cross-cutting/core-module.md

## Summary

The document substantially matches the implementation and covers all required sections. Its main factual issue is conflating the numerics output-directory layout with API visibility: the differentiable annex is intended for clients to import. The document also exceeds its per-document prompt’s size limit.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show cross-cutting/core-module.md`.
- Verified against the **current working tree**, which the operator states equals the document’s source commit for all of `source/`; the source-tree diff against that commit was empty.
- Spot-checked more than 20 claims, including scalar aliases, modifier declarations, matrix-layout parameters, floating-point matrix/vector multiplication, vector `dot` and `length`, GLSL loading, standard-module paths and compiler selection, numerics artifact dependencies, functional operations, experimental-module validation, prelude registration, command-line prelude overrides, core-module generation, and cache layout.
- Verified all **six line-number citations**: `source/slang/core.meta.slang:2362,2372`; `source/slang/slang-session.cpp:43,73,1651`; and `source/slang/slang-diagnostics.lua:603`. All match.
- Resolved all **72 Markdown link occurrences**, comprising 52 distinct destinations, against the working tree; checked both local heading anchors and manifest membership for all three linked generated pages.
- Consulted `architecture/overview.md` only for cross-reference context.
- Checked required headings, coverage of all four watched meta-Slang files and nine prelude headers, front-matter keys and formats, relative-link style, and absence of raw Liquid openers. Measured the document at **28,821 UTF-8 bytes**.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Standard modules`, lines 211–219 and 268–273 | Calling the numerics artifacts `one public and three private` incorrectly includes `differentiable.slang-module` among private implementation modules. Likewise, saying the adapters are `kept out of the public import graph` misstates the dependency direction: public modules import the adapters; the adapters avoid importing public modules. | `source/standard-modules/numerics/differentiable-wrappers.slang:11–14` explicitly describes clients importing `slang.numerics.differentiable`. `source/standard-modules/numerics/differentiable.slang:11–12` imports both the base module and its adapter. `source/standard-modules/numerics/CMakeLists.txt:59–63` states: “The public modules import these adapters.” | Describe two public entry points, `slang.numerics` and `slang.numerics.differentiable`, plus two implementation adapters. Preserve the accurate output paths, but clarify that directory nesting does not establish privacy. Replace the import-graph claim with the precise statement that adapters do not import the public numerics modules. |
| F-002 | minor | Entire document | At 28,821 bytes, the document exceeds the per-document requirement to remain under 24 KB, although it satisfies the manifest’s larger 32,768-byte cap. | `docs/generated/design/_meta/prompts/cross-cutting-core-module.md:53`: `Document length under 24 KB.` | Reduce the document below 24,000 UTF-8 bytes to satisfy either interpretation of KB. Condense repeated build/cache explanations and the detailed standard-module file inventory while retaining the required sections and coverage of every meta-Slang file and prelude header. |