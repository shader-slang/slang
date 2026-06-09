---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:15:42+00:00
target_doc: pipeline/03-semantic-check.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: a7f01f5c13a93b4962311b4a8303731a575df2231fa88c54d62f0ee4ce433cb4
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 1
  minor: 0
  nit: 0
---

# Review report for pipeline/03-semantic-check.md

## Summary
Two findings were identified. The main issue is that the file-responsibility table classifies `slang-check-out-of-bound-access.cpp` as part of semantic checking even though the source implements it as an IR pass invoked from `slang-emit.cpp`; the page also misses a prompt-required `slang-ast-decl-ref.cpp` citation in the `DeclRef` section.

## Items checked
- Ran `regenerate.py show pipeline/03-semantic-check.md` and reviewed the manifest entry, prompt, resolved watched files, and dependency on `pipeline/02-parse-ast.md`.
- Verified front matter fields and resolved all 37 relative links.
- Checked `checkTranslationUnit`, `SemanticsVisitor : public SemanticsContext`, `DiagnosticSink` threading, the watched `slang-check-*.cpp` responsibility table, and parser interaction through `parseUnparsedStmt`.
- Spot-checked name-resolution and `DeclRef` references, generic constraint and conformance files, synthesis references, modifier and shader-specific sections, and failure-mode claims about continued checking after diagnostics.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Name lookup and DeclRef`, lines 88-97 | The prompt requires this section to cite `slang-ast-decl-ref.cpp`, but the section links only the generated name-resolution index and handwritten `decl-refs.md`. This leaves out the requested implementation pointer for `DeclRef` mechanics. | `docs/generated/design/_meta/prompts/pipeline-03-semantic-check.md:38-41` requires citing `slang-ast-decl-ref.cpp`; `source/slang/slang-ast-decl-ref.cpp:11` starts the `DirectDeclRef` implementation and `source/slang/slang-ast-decl-ref.cpp:164` starts `LookupDeclRef` substitution. | Add a workspace-relative link to `source/slang/slang-ast-decl-ref.cpp` in the `DeclRef` paragraph, with a short note that it owns the concrete `DeclRefBase` operations. |
| F-002 | critical | `## SemanticsVisitor`, lines 55-70 | The responsibility table describes `slang-check-out-of-bound-access.cpp` as part of the semantic-checking file family, but the source implements it as an IR module pass, not a `SemanticsVisitor` checker. This places a post-lowering validation in the wrong pipeline stage. | `source/slang/slang-check-out-of-bound-access.cpp:12` defines `OutOfBoundAccessChecker : public InstPassBase`; `source/slang/slang-check-out-of-bound-access.cpp:101` takes an `IRModule*`; `source/slang/slang-emit.cpp:1380` invokes it through `SLANG_PASS(checkForOutOfBoundAccess, sink)`. | Remove this row from the semantic-checker responsibility table, or explicitly move the out-of-bounds check discussion to the IR-pass or target-pipeline documentation. |
