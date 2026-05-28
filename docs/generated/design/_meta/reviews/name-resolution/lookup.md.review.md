---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: name-resolution/lookup.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 0f6a6813ac215b2f02942be90ab761d8979a0ae51be857c45bd14bd32a88b0d0
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for name-resolution/lookup.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked lookup entry points, `LookupMask`, `LookupOptions`, breadcrumb kinds, unqualified/member lookup steps, transparent-member injection, shadowing rules, and edge cases.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Algorithm` and edge cases | Required deduplication behavior is not covered, and source `AddToLookupResult` appends results without deduping by `DeclRef`. | `docs/generated/design/_meta/prompts/name-resolution-lookup.md` requires deduplication coverage; `source/slang/slang-lookup.cpp:95-125` appends lookup items. | Document the actual no-dedupe accumulation behavior or identify the real dedupe site if elsewhere in watched files. |
| F-002 | major | multiple sections | The page cites non-watched files despite the name-resolution watched-path rule. | The `lookup.md` manifest entry excludes cited files such as `slang-ast-decl.cpp`, `slang-check-inheritance.cpp`, `slang-check-stmt.cpp`, `slang-check-decl.cpp`, and `slang-diagnostics.lua`. | Expand watched paths or remove/replace those citations with watched-file evidence. |
