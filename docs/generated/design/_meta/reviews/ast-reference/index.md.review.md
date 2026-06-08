---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:18:11+00:00
target_doc: ast-reference/index.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: cbccfa6aafcc9d4307bb48a229affc8e0177c1211a2813463404fd1e5ba4f6be
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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

# Review report for ast-reference/index.md

## Summary
No findings were identified for the AST reference index. The page satisfies the navigation-page prompt, resolves its links, and its approximate family counts are within the required tolerance.

## Items checked
- Ran `regenerate.py show ast-reference/index.md` and used the listed prompt, dependency docs, and watched paths.
- Verified the target front matter fields and checked the source-backed taxonomy against `source/slang/slang-ast-base.h`.
- Resolved all 38 Markdown links and anchors in the target document.
- Checked the `## Pages` approximate counts against concrete FIDDLE class counts: declarations 60, expressions 91, statements 30, types 119, values 60, modifiers 254.
- Checked the required two-paragraph intro, family taxonomy, pages table, cross-cutting topics, and navigation guidance sections. The document has no source line-number citations in the body.

## Findings

(no findings)

## No-issues notes
- The taxonomy diagram uses declared base roots from `slang-ast-base.h`, including `NodeBase`, `SyntaxNodeBase`, `SyntaxNode`, `ModifiableSyntaxNode`, `DeclBase`, `Decl`, `Expr`, `Stmt`, `Modifier`, `Val`, `Type`, and `DeclRefBase`.
- The page remains a navigation hub and does not duplicate per-node reference material from the family pages.
