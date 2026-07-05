---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:04:26Z
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/scopes.md

## Summary
The review raised one minor finding (F-001) claiming the body ends with leaked wrapper tags on lines 372-373. Verification against the target document at HEAD shows it is 371 lines, ends cleanly on the glossary See also bullet, and contains no such tags anywhere. The finding is rejected as bogus; no edit was made, so the document front-matter is unchanged.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The file docs/generated/design/name-resolution/scopes.md is 371 lines (wc -l reports 371), so lines 372-373 do not exist. A grep for the closing content, invoke, and antml tags returns exit 1 with no matches, and tail shows the file terminating on the glossary bullet naming scope, decl-ref, lookup result, name resolution. No wrapper tags are present to remove. | — |
