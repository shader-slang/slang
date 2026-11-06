---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:20:25Z
target_doc: pipeline/02-parse-ast.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 4246bb60e01045efde01e27a0e773a37c259f70db6c6e96b5162994eb761d09a
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 0
---

# Review report for pipeline/02-parse-ast.md

## Summary

The document covers the required topics and largely matches the implementation, but needs three corrections. The most important issue is its assertion that typedef and underlying-type spellings directly share a `Type*`: the source preserves named types and canonicalizes them separately. The error-recovery description also overstates its coverage, and one line citation is stale.

## Items checked

- Reviewed against the **current working tree**, which the operator states equals the document’s source commit for all of `source/`.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show pipeline/02-parse-ast.md` to obtain the manifest entry and resolved watched paths.
- Spot-checked more than 20 claims: both parser entry signatures, deferred bodies and scopes, parsing stages, syntax lookup and callbacks, generic disambiguation and FOLLOW tokens, annotation skipping, error recovery, AST family inheritance, casting, builder interning, constraint forms, associated-type constraint placement, modifier version gates, declaration nesting, and integer/floating-point literal handling.
- Checked every explicit line-number citation, including ranges and citations that inherit their filename from surrounding prose.
- Resolved all 55 Markdown links, covering 27 distinct destinations; checked target-file existence at the recorded source commit, generated-page manifest membership, and both fragment targets. Read dependency and peer passages only for cross-reference checks.
- Verified required sections, audience introduction, front-matter keys and formats, relative-link style, and absence of raw Liquid openers. The document is 32,579 bytes, below its 32,768-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### ASTBuilder`, lines 375–382 | The explanation that `float3` and `vector<float, 3>` “intern to the same `Type*`” conflates interning with canonicalization. A typedef spelling can retain a separate `NamedExpressionType`; the identical-signature conclusion does not establish raw pointer identity. This gives compiler developers an incorrect invariant for comparing types. | `source/slang/slang-syntax.cpp:958–963`: `getNamedType` interns a `NamedExpressionType` keyed by the typedef declaration reference. `source/slang/slang-ast-type.cpp:1400–1405` separately resolves that named type to its underlying canonical type. `source/slang/slang-ast-base.h:586` exposes `getCanonicalType()` through `resolve()`. | Preserve the redeclaration example, but explain that the spellings denote the same canonical type. Explicitly distinguish interning identical node descriptions from canonicalizing typedef wrappers; remove the claim of direct pointer identity between these spellings. |
| F-002 | minor | `### Error recovery`, lines 233–255 | “Every other recovery site goes through `TryRecoverBefore`” omits the identifier-text overload of `ReadToken`. The conclusion that recovery outside blocks targets a closing token “rather than … a separator” is also too broad: `readTokenImpl` forwards whatever token was expected. | `source/slang/slang-parser.cpp:479–515`: `Parser::ReadToken(const char*)` performs its own balanced-token recovery loop until the requested identifier or a closing token. At `source/slang/slang-parser.cpp:703`, `readTokenImpl` calls `TryRecoverBefore(this, expected)` without restricting `expected` to closing tokens. | Scope the two-site inventory specifically to callers of `TryRecoverBefore`. Add the identifier-text overload’s separate recovery behavior, and state that `readTokenImpl` can recover before any expected token, including a separator; matched-region recovery targets the region’s closing token. |
| F-003 | minor | `## Modifier parsing`, lines 495–497 | The citation places `ParseModifiers` at line `1237`, which is inside a different helper’s diagnostic branch. | `source/slang/slang-parser.cpp:1237` contains `SLANG_DIAGNOSE_UNEXPECTED`; `ParseModifiers` begins at `source/slang/slang-parser.cpp:1279`, as the document itself correctly states earlier. | Change this citation from `1237` to `1279`. |