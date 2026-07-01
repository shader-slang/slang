---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:00:16Z
target_doc: cross-cutting/serialization.md
review_report: ../../reviews/cross-cutting/serialization.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 3
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/serialization.md

## Summary
All three findings were rejected as bogus: the current target document already matches the watched source on every cited point, so no edits were made this cycle. F-001 claims the doc names the mode type `SerializerMode`, but the doc already uses the correct `SerializationMode`. F-002 claims the source-location section says file content and an expansion stack are serialized, but the doc explicitly states the file content is not serialized and lists only path/range/line tables. F-003 claims the embedded-core-module rationale is unsupported, but it is paraphrased from the fossil header comment. The review reads as written against an earlier draft than the document present at this commit.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc line 68 reads "distinguished by a `SerializationMode`"; grep finds no `SerializerMode` anywhere in `docs/generated/design/cross-cutting/serialization.md`. Matches `source/slang/slang-serialize.h:62` (`enum class SerializationMode`) and `:122` (`getMode()`). The claimed mismatch does not exist. | — |
| F-002 | rejected-bogus | Doc lines 201-214 state the serializer captures per-file "path, its source-location range, and its line-start tables (both unadjusted and `#line`-adjusted)" and that "the file's content is not serialized," reconstructing via `createSourceFileWithSize`. Matches `source/slang/slang-serialize-source-loc.h:74-84` and `slang-serialize-source-loc.cpp:270-277`. The doc never claims content or an expansion stack is serialized. | — |
| F-003 | rejected-bogus | The fossil header comment `source/slang/slang-serialize-fossil.h:29-35` says the toggle exists because "one of the key cases for serialization in Slang is loading the core module from the `slang.dll` binary itself ... we provide a define ... to measure how much performance is being lost to validation checks." Doc lines 126-133 paraphrase this; it is not invented. The finding cited only lines 36-44 and overlooked the comment. | — |
