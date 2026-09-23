---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T15:10:00Z
target_doc: syntax-reference/tokens.md
review_report: ../../reviews/syntax-reference/tokens.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/tokens.md

## Summary

Six minor findings were reviewed. Five were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed: the source inventory now lists `slang-preprocessor.cpp`, the punctuation "dispatch table" is now the per-character switch it actually is, `AfterWhitespace` names its real consumers, the numeric-suffix bullet is trimmed back to the structural fact the prompt allows, and the source-location paragraph describes the real `SourceLoc` / `SourceLocType` / `SourceView` model. The front-matter digest finding was rejected as out of scope. The document was edited.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-preprocessor.cpp` is in this page's watched set, it owns the `<...>` include-path assembly in `HandleIncludeDirective`, and it is the reader of the token flags the page documents. The prompt's `## Source` clause asks the page to name its watched paths. | `## Source`: added a `slang-preprocessor.cpp` bullet naming `HandleIncludeDirective`. |
| F-002 | fixed | `_lexTokenImpl` at `source/compiler-core/slang-lexer.cpp:1966` opens `switch (nextCodePoint)` with per-character arms (for example the `'+'` arm at `:2181`); there is no punctuation table. | `### Punctuation and structural symbols`: "punctuation dispatch table" replaced with the per-character `switch` in `_lexTokenImpl`. |
| F-003 | fixed | In `source/slang/slang-preprocessor.cpp` the flag is read at `:2536` (spacing during stringization), `:3901` (function-like macro recognition via a `(` with no preceding whitespace), `:4106`, `:4972`, and `:5151`; the token-paste code does not read it. | `## Token flags`: `AfterWhitespace` row now names stringization spacing and function-like macro recognition. |
| F-004 | fixed | `docs/generated/design/_meta/prompts/syntax-tokens.md` lines 42-46 scope this bullet to "only the structural fact that suffixes are part of the token text and parsed later"; the decoder detail exceeded that. | `## Special-case lexing rules`: numeric-suffix bullet truncated after the raw-text/deferred-decoding statement. |
| F-005 | fixed | `source/compiler-core/slang-source-loc.h:388-394` defines `SourceLocType` as `Nominal` / `Actual` / `Emit`, and `:532` plus `:560` show the initiating location is a `SourceView` field reached via `getInitiatingSourceLoc`, not part of the 32-bit encoding. | `## Source location`: spelling/expansion sentence replaced with the `SourceLocType` interpretations and the separate `getInitiatingSourceLoc`. |
| F-006 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run: "Do not edit those three fields yourself." The digest is refreshed when the operator marks this page fresh after the edits above. | — |
