---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:40:52Z
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 686b8730f14d503c2c40ba8ae76a5c904f438901aace8656656d58178d874fee
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
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

# Review report for syntax-reference/keywords-and-builtins.md

## Summary

The document substantially satisfies its generation contract, and its keyword tables and line-number citations match the implementation. Two minor corrections are needed in the explanation of syntax declarations: user-source examples exist in the repository, and alias declarations inherit the existing syntax class only conditionally.

## Items checked

- Reviewed against the **current working tree**, which matches the document’s source commit for `source/`, as specified by the operator.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the watched-path listing from `regenerate.py show`.
- Checked every explicit line-number citation in the target document against `slang-parser.cpp`, `core.meta.slang`, `hlsl.meta.slang`, and `slang-ir-legalize-varying-params.cpp`; no materially displaced citations found.
- Spot-checked more than 20 behavioral claims, including syntax registration, syntax aliases, statement dispatch, `$for` version checks, `case`/`default` placement, chained catches, class/struct parsing, function-type syntax, modifier callbacks, differentiation operands, shape-operation arities, descriptor-heap declarations, and reserved-name checks.
- Resolved all 33 relative-link occurrences, covering 12 distinct destinations; all five linked generated pages appear in the manifest. Read the dependency `tokens.md` only for cross-reference checks.
- Checked required sections, front-matter keys and formats, Markdown structure, and Liquid safety. The document is 31,313 bytes, below its 32,768-byte cap.
- Made no file changes.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Parser-registered syntax keywords`, lines 73–75 | The assertion that `no user-level example exists in the tree` and that the only instances are core-module source is false. Regression tests explicitly declare and use custom syntax in user source. | `tests/diagnostics/redeclaration-user-alias-modifier.slang:6` declares `syntax my_alias : UnscopedEnumCaseAliasModifier;`. `tests/diagnostics/redeclaration-user-buffer-modifiers.slang:7–8` declares two more custom modifiers and uses them on a variable. | Remove the exclusivity claim. State that core modules use these declarations and that user-source regression tests also exercise them; cite one of these tests. |
| F-002 | minor | `## Parser-registered syntax keywords`, lines 81–83 | The statement that the alias form `copies the named keyword's parse callback and syntax class` omits the explicit-class exception. The callback and user data are copied, but an explicitly supplied syntax class is preserved. | `source/slang/slang-parser.cpp:5522–5529` copies `parseCallback` and `parseUserData`, then assigns the existing class only under `if (!syntaxClass)`. | Say that the alias form copies the callback and user data, and inherits the existing syntax class only when no explicit class was supplied. Qualify the “drop-in alias” description accordingly. |