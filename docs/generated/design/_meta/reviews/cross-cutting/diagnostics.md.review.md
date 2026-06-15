---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:05+00:00
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: d42270f6daaebc67791d018218ab19aef406d36f8b4094dd0346c2a1ef697ec1
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 0
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 0
---

# Review report for cross-cutting/diagnostics.md

## Summary
The diagnostics page satisfies the prompt contract and the sampled source claims align with the recorded source commit. I found no reportable factual, structural, link, style, or front-matter issues.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/diagnostics.md` and reviewed the per-doc prompt, `_common.md`, the resolved watched files, and dependency `architecture/overview.md`.
- Checked front matter for all required generated-doc keys, the recorded source commit, digest shape, and warning string.
- Verified required coverage for `DiagnosticSink`, diagnostic definitions, severity levels, source-location rendering, error code namespace, internal compiler errors, and the new-diagnostic checklist.
- Spot-checked more than 10 source claims against `slang-diagnostic-sink.h`, `slang-diagnostic-sink.cpp`, `slang-diagnostics.h`, `slang-diagnostics.lua`, `diagnostics/type-errors.lua`, `slang-common.h`, and `slang-signal.cpp`.
- Checked relative links and peer links used by the page; no unresolved target was found.
- Checked that the body has no source line-number citations requiring line-by-line verification.

## Findings
(no findings)

## No-issues notes
- The `Severity` enum and rendered severity names match `source/compiler-core/slang-diagnostic-sink.h`.
- The rich and legacy diagnostic examples match `source/slang/diagnostics/type-errors.lua` and `source/slang/slang-diagnostics.lua`.
- The assertion paragraph now distinguishes sink-based internal-error diagnostics from `SLANG_ASSERT` / `SLANG_RELEASE_ASSERT` handling.
