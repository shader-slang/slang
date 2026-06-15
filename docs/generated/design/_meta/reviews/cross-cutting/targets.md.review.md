---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:05+00:00
target_doc: cross-cutting/targets.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 84097e639319e025582296c205ef440d38bd023139ac79f25b4042f2e2d3f2d4
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

# Review report for cross-cutting/targets.md

## Summary
The targets page satisfies the prompt contract and the sampled source claims align with the recorded source commit. I found no reportable factual, structural, link, style, or front-matter issues.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/targets.md` and reviewed the per-doc prompt, `_common.md`, resolved watched files, and dependencies `architecture/overview.md` and `pipeline/06-emit.md`.
- Checked front matter for required keys, source commit, watched-path digest, and warning string.
- Verified required coverage for target rows, capability-system vocabulary, profiles, target-sensitive IR behavior, per-target pipelines, and the add-target checklist.
- Spot-checked more than 10 source claims against `include/slang.h`, `slang-emit-*`, `slang-capabilities.capdef`, `slang-capability.h`, `slang-capability.cpp`, `slang-profile.h`, `slang-profile.cpp`, `slang-profile-defs.h`, and `slang-target.cpp`.
- Checked that target rows point at existing emit files and that the `raytracing` capability example matches the current alias declaration.
- Checked relative links and peer links used by the page; no unresolved target was found.
- Checked that the body has no source line-number citations requiring line-by-line verification.

## Findings
(no findings)

## No-issues notes
- The public target table matches the sampled `SlangCompileTarget` enum entries in `include/slang.h`.
- The capability vocabulary aligns with the introductory comments and declarations in `source/slang/slang-capabilities.capdef`.
- The profile discussion matches `Profile`, `Stage`, and profile-definition X-macros in `source/slang/slang-profile.*`.
