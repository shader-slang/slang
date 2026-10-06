---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:20:00Z
target_doc: ir-reference/index.md
review_report: ../../reviews/ir-reference/index.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/index.md

## Summary
All three findings were fixed, and the document was edited (so `mark-fresh`
is needed). The page is back inside its navigation-only contract: it shrank
from 21,237 to 10,346 bytes by dropping the per-opcode, producer-migration,
and wrapper-generation audits that belong on the family pages, the `## Pages`
table now carries the required **Approx. opcodes** column with nearest-ten `~`
values, and the taxonomy groups its ten family pages under five intermediate
nodes. The two stale factual paragraphs the reviewer flagged (F-002, F-003)
were removed as part of the same trim rather than corrected in place, which is
what both recommendations asked for.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against `docs/generated/design/_meta/prompts/ir-reference-index.md`: item 2 asks for a two-paragraph intro, item 3 says "don't put every family directly under `IRInst`", item 4 fixes the column as **Approx. opcodes** rounded to the nearest ten with a `~` prefix, items 5 and 7 cap those sections at three or four sentences, and the checklist at lines 72-73 forbids describing a family page's opcodes in detail here. The page violated all five. Counts use the nearest ten of each page's verified row count (types 170, values 148, structure 23, control-flow 26, generics 59, resources 90, differentiation 39, decorations 196, metadata 59, misc 65); generics is written `~50` because seven of its 59 rows are cross-links, so the Lua-entry count is about 52. See also F-002, F-003. | Deleted the intro's abstract-entry paragraph; regrouped the taxonomy into five intermediate nodes (each family page still a leaf exactly once); renamed **Table rows** to **Approx. opcodes** and rounded all ten values; replaced the row-count methodology, cross-link tally, and "most recent additions" prose with a two-sentence note; collapsed `## How AST nodes lower to IR` and `## How to navigate` to four sentences each; deleted the `### The C++ wrapper column` subsection. |
| F-002 | fixed | Confirmed stale. `docs/generated/design/ir-reference/resources-and-atomics.md` marks nine no-producer rows and `docs/generated/design/ir-reference/values.md` five (`castToVoid`, `PtrCast`, `getAddr`, `alloca`, `ReinterpretOptional` — the last one added by that page's own remediation this cycle), not seven and one. Took the reviewer's first option: a per-page no-producer tally is exactly the kind of sibling-page detail the index checklist excludes, and keeping it would guarantee the same drift next cycle. | Deleted the paragraph carrying the counts; the retained sentence in `## How AST nodes lower to IR` now only defines the **no producer at HEAD** marker without tallying pages. |
| F-003 | fixed | Confirmed obsolete. `python3 docs/generated/design/_meta/regenerate.py show ir-reference/index.md` resolves eight watched source paths (`slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h.lua`, `slang-lower-to-ir.cpp`, `slang-ast-expr.h`, and the three meta-module files) plus the sibling-doc glob, so the paragraph's premise and its "should be added" request are both false now. Did not substitute another manifest note, per the recommendation. | Deleted the closing manifest-coverage paragraph. |
