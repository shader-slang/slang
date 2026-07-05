---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:31:00+00:00
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: f75116b2323ea549005589e147bdc7c2b79a63ace127358d8904fccc95cf2272
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for syntax-reference/keywords-and-builtins.md

## Summary
The page is mostly source-aligned, but the keyword inventory is incomplete. The main issue is that the hardcoded parser vocabulary includes internal statement keywords and type-specifier keywords that are not mentioned in the document, even though the prompt asks for an inventory of syntactic keywords and built-in syntax declarations.

## Items checked
- Ran `regenerate.py show syntax-reference/keywords-and-builtins.md` and reviewed the target document, `_common.md`, the per-document prompt, the dependency document `syntax-reference/tokens.md`, and the resolved watched-file set.
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the relative markdown links in the body to source files or generated peer documents.
- Spot-checked source-backed claims against watched files, including `TokenType::Identifier`, `SyntaxDecl`, `LookAheadToken("if")`, `LookAheadToken("for")`, `parseCompileTimeForStmt`, `parseIfStatement`, `g_parseSyntaxEntries[]`, `_makeParseDecl`, `_makeParseModifier`, `_makeParseExpr`, `getSyntaxParseInfos()`, `struct`/`class`/`enum` special-casing, `new`, `Optional`, `Tuple`, `Texture2D`, `WaveGetWaveIndex`, `vec3`, `mat4`, `gl_Position`, and `DifferentialPair`.
- Checked the document's required sections against `syntax-keywords-and-builtins.md` and the universal generated-doc contract.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Parser-registered syntax keywords`, statement/type-specifier inventory | The inventory omits several hardcoded parser keywords. The statement table lists ordinary control-flow keywords but not internal statement forms such as `__target_switch`, `__stage_switch`, `__intrinsic_asm`, and `__GPU_FOREACH`; the decl/type discussion calls out `struct`, `class`, and `enum` but not the hardcoded type-specifier forms `expand` and `each`. | `source/slang/slang-parser.cpp:6939`-`source/slang/slang-parser.cpp:6953` directly dispatches `__target_switch`, `__stage_switch`, `__intrinsic_asm`, and `__GPU_FOREACH`; `source/slang/slang-parser.cpp:3435`-`source/slang/slang-parser.cpp:3441` directly recognizes `expand` and `each` alongside the shape expression forms. | Add these hardcoded parser keywords to the appropriate parser-keyword discussion, marking the double-underscore / GPU forms as internal or experimental where appropriate. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
