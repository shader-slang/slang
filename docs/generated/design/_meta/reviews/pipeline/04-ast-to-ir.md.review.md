---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:04:49+00:00
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: d6bb63223a0f2c41cb6dae09f1e45a4ddd44111fc422e5baf80f5ec043e38517
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

# Review report for pipeline/04-ast-to-ir.md

## Summary

The AST-to-IR page matches its lowering prompt and the sampled lowering sources. I found no findings.

## Items checked

- Ran `regenerate.py show pipeline/04-ast-to-ir.md` and read the target page, `_common.md`, `pipeline-04-ast-to-ir.md`, dependencies `pipeline/03-semantic-check.md` and `cross-cutting/ir-instructions.md`.
- Verified front matter keys, recorded source commit, and 64-character hex watched-path digest.
- Spot-checked 15 lowering claims against `source/slang/slang-lower-to-ir.h`, `source/slang/slang-lower-to-ir.cpp`, `source/slang/slang-ir.h`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir-insts.h`, and `source/slang/slang-ir-insts.lua`.
- Checked cited lowering symbols including `generateIRForTranslationUnit`, `generateIRForSpecializedComponentType`, `generateIRForTypeConformance`, `getInterfaceRequirementKey`, `IRBuiltinRequirementDecoration`, `visitGenericTypeConstraintDecl`, and `Diagnostics::UnsupportedAssignmentTarget`.
- Resolved relative links with `regenerate.py lint` for this assigned doc group; lint reported no issues.

## Findings

(no findings)

## No-issues notes

- The page correctly keeps opcode details and pass ordering out of scope and links to the dedicated pages.
- The built-in requirement-key discussion is supported by the `getInterfaceRequirementKey` implementation and cache.
- The adjacent pipeline notes match the pre-link and layout-IR pages.
