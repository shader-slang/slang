---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:05+00:00
target_doc: cross-cutting/serialization.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 8b042e75fd998180a0b911649454d28b10dc08df645aa95b3ce5e8eb390b7f82
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

# Review report for cross-cutting/serialization.md

## Summary
The serialization page satisfies the prompt contract and the sampled source claims align with the recorded source commit. I found no reportable factual, structural, link, style, or front-matter issues.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/serialization.md` and reviewed the per-doc prompt, `_common.md`, resolved watched files, and dependency `architecture/overview.md`.
- Checked front matter for required keys, source commit, watched-path digest, and warning string.
- Verified required coverage for serialized payloads, generic serialization, fossil, RIFF containers, source-location serialization, versioning, round-trip/repro notes, and adding serialized fields.
- Spot-checked more than 10 source claims against `slang-serialize.h`, `slang-serialize-container.cpp`, `slang-serialize-fossil.h`, `slang-serialize-ir.cpp`, `slang-serialize-ir.h`, `slang-serialize-riff.cpp`, `slang-serialize-source-loc.cpp`, and `slang-ir.h`.
- Confirmed that every serialize-related file listed in the resolved watched paths is mentioned in the page.
- Checked relative links and peer links used by the page; no unresolved target was found.
- Checked that the body has no source line-number citations requiring line-by-line verification.

## Findings
(no findings)

## No-issues notes
- The fossil validation description matches `SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS` and `SLANG_SERIALIZE_FOSSIL_VALIDATE`.
- The flat IR read-path description matches `deserializeFromFlatModule`, including the `Unrecognized` skip for literal/string-consumption assertions.
- The module-version note matches the `IRModule` version constants and nearby comments in `source/slang/slang-ir.h`.
