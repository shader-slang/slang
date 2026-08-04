---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:40:00+00:00
target_doc: syntax-reference/tokens.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 835ca8ae0e597a608474aaa8d581c75de3d392cc320259f50ca47f254b43433d
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for syntax-reference/tokens.md

## Summary
The token catalog is largely complete and matches the `TokenType` enumerators in `slang-token-defs.h`. The one issue found is a source-alignment problem in the `CompletionRequest` row: it cites a non-existent source file and misses the actual watched lexer branch that emits `#?`.

## Items checked
- Ran `regenerate.py show syntax-reference/tokens.md` and reviewed the target document, `_common.md`, its per-document prompt, and the resolved watched-file set (`source/compiler-core/slang-lexer.cpp`, `source/compiler-core/slang-lexer.h`, `source/compiler-core/slang-token-defs.h`, `source/compiler-core/slang-token.cpp`, `source/compiler-core/slang-token.h`).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Compared every token row in the taxonomy against `TOKEN(...)` / `PUNCTUATION(...)` entries in `slang-token-defs.h`.
- Resolved the relative markdown links in the body to source files or generated peer documents.
- Spot-checked source-backed claims and named symbols against the watched files, including `Token`, `TokenType`, `TokenFlags`, `TokenFlag::AtStartOfLine`, `TokenFlag::AfterWhitespace`, `TokenFlag::ScrubbingNeeded`, `TokenFlag::Name`, `_lexTokenImpl`, `Lexer::lexToken`, `_lexStringLiteralBody`, `_lexRawStringLiteralBody`, `_maybeLexNumberExponent`, `getStringLiteralTokenValue`, `getCharLiteralValue`, `kLexerFlag_SuppressDiagnostics`, `Lexer::getDiagnosticSink`, and the `#?` `CompletionRequest` branch.
- Checked the prompt-required sections: source list, token taxonomy, token flags, special-case lexing rules, source-location paragraph, and keyword deferral note.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Preprocessor markers`, `CompletionRequest` row | The row says `CompletionRequest` is "synthesized by the language-server pipeline" and cites `slang-completion-token.cpp`, but that file does not exist in the repository and the watched lexer source directly emits `TokenType::CompletionRequest` for `#?`. | `source/compiler-core/slang-lexer.cpp:2129`-`source/compiler-core/slang-lexer.cpp:2139` handles `#` followed by `?` and returns `TokenType::CompletionRequest`; no `source/**/slang-completion-token.cpp` file exists. | Change the row's source range to the `#` branch in `_lexTokenImpl` and remove the non-existent `slang-completion-token.cpp` citation. If the language-server insertion behavior is retained, cite the actual file and add it to the manifest's watched paths. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
