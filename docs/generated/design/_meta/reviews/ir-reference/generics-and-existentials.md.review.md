---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:37+00:00
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: fail
finding_count: 6
severity_breakdown:
  critical: 0
  major: 2
  minor: 2
  nit: 2
---

# Review report for ir-reference/generics-and-existentials.md

## Summary

The page is broadly accurate and all source citations and links resolve, but it does not satisfy the mandatory producer-column contract: 40 opcode rows retain retired `(synthesized)` or bare `—` origins. Its recorded watched-path digest also does not match the current manifest entry.

## Items checked

- Inspected the target, both generation prompts, all 11 resolved watched files, and all five dependency documents.
- Verified all 83 unique source line targets against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`; every cited line matched.
- Spot-checked more than 25 factual claims, including schemas, wrappers, producers, existential projections, witness lookup, RTTI, sets, tag operations, and tagged-union operand order.
- Resolved all 78 relative-link occurrences (34 unique targets) and confirmed every generated-doc peer is present in the manifest.
- Swept 278 identifier-like tokens and 18 source filenames, checked front matter and required sections, and confirmed the document lints cleanly.

## Findings

| ID    | Severity | Location                                                           | Description                                                                                                                                                                                                                                                                       | Evidence                                                                                                                                                                                                                                                           | Recommendation                                                                                                                                       |
| ----- | -------- | ------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `## Opcodes`, lines 178-355                                        | Forty six-column opcode rows use the retired `(synthesized)` catch-all or bare `—` in `AST origin`, including rows that already name a producing function. Four unproduced opcodes also use `—` instead of the required **no producer at HEAD** wording.                          | `docs/generated/design/_meta/prompts/_common.md:240-241` requires the actual pass/function or **no producer at HEAD**; examples include producers at `source/slang/slang-ir-legalize-global-values.cpp:229` and `source/slang/slang-ir-bind-existentials.cpp:350`. | Replace every retired label with the specific producer. For each truly uncalled opcode, write **no producer at HEAD** and state that in its summary. |
| F-002 | major    | Front matter, line 6                                               | `watched_paths_digest` is `64be22...`, but the current manifest entry resolved by `regenerate.py show` includes 11 watched files and `regenerate.py digest ir-reference/generics-and-existentials.md` returns `5016fe2773a7e78a94911faa961b75c8f140bba84ba007a0f0e81fc088522624`. | `docs/generated/design/_meta/manifest.yaml:637-653`; target front matter line 6.                                                                                                                                                                                   | Regenerate the page from the current manifest inputs so its front matter records the current digest.                                                 |
| F-003 | minor    | `## Source`, lines 98-101; `UnboundedGenericElement` row, line 315 | The claim that exactly four opcodes have no producer omits `UnboundedGenericElement`. Its builder exists, but no source call site invokes it, matching the page's own criterion for the other four unproduced opcodes.                                                            | `source/slang/slang-ir-insts.h:4610-4613` defines `getUnboundedGenericElement`; the only other source references are consumer/classification cases at lines 2983/2999 and `source/slang/slang-ir.cpp:9691`.                                                        | Add `UnboundedGenericElement` to the no-producer inventory and mark its row **no producer at HEAD**.                                                 |
| F-004 | minor    | `## Opcodes`, lines 246-252; `specialize` callout, lines 369-375   | The page describes specialization-pass behavior—when `lookupWitness` is replaced and how `specialize` is replaced by a generic's result—even though the per-document prompt forbids specialization-pass details.                                                                  | `docs/generated/design/_meta/prompts/ir-reference-generics-and-existentials.md:70-73`.                                                                                                                                                                             | Keep only opcode-level semantics and refer readers to the existing `pipeline/05-ir-passes.md` link for pass behavior.                                |
| F-005 | nit      | Introduction, lines 12-21                                          | The opening paragraph states the scope, but the intended reader is deferred to a second paragraph; the common contract requires both in the first paragraph.                                                                                                                      | `docs/generated/design/_meta/prompts/_common.md:65-66`.                                                                                                                                                                                                            | Merge the intended-reader sentence into the opening paragraph.                                                                                       |
| F-006 | nit      | `interface_req_entry` row, line 231                                | The phrase `an associated-type bound such as` is duplicated verbatim.                                                                                                                                                                                                             | Target document line 231.                                                                                                                                                                                                                                          | Remove one copy of the duplicated phrase.                                                                                                            |
