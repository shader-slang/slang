---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:06+00:00
target_doc: ir-reference/index.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: b01105947bb6bdcf6a24a6d12b46521c4b6bfb52a24e7ee5da31dceb7f981082
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for ir-reference/index.md

## Summary
The index is extensively sourced, and its links, front matter, and line-number citations check out. However, it substantially exceeds the navigation-only contract and uses the wrong `## Pages` table format. Two factual summaries have also drifted from the current dependency pages and manifest.

## Items checked
- Ran `regenerate.py show ir-reference/index.md` and inspected the target, both generation prompts, all ten dependency pages, and every resolved watched file.
- Verified all 43 distinct source line anchors and range endpoints against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including the Lua roots, the three lowering visitors, and the `slang-ir.h.lua` generation sites.
- Spot-checked more than ten factual claims, including the Lua file length, family row counts, placeholder counts, wrapper-prefix and marker counts, `InfixExpr` lowering, and wrapper generation.
- Resolved all 33 unique relative link targets and their explicit anchors in the current dependency pages.
- Checked the required headings and table contract, the taxonomy coverage, the front-matter keys, the warning text, and the hexadecimal digest shape.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Intro and `## Family taxonomy` through `## How to navigate`, lines 12-335 | The page substantially violates its navigation-only generation contract. The intro has three paragraphs instead of two; the taxonomy puts every family directly under `IRInst` instead of grouping siblings; `## Pages` replaces **Approx. opcodes** with exact **Table rows** values; and the lowering, navigation, opcode-addition, producer-audit, and wrapper-audit material expands the requested short guidance into hundreds of lines of per-opcode detail. | `docs/generated/design/_meta/prompts/ir-reference-index.md:19-64` requires the short structure, rounded `~` counts, grouped taxonomy, and 3-4 sentence guidance; lines 72-73 explicitly forbid detailed opcode description in this index. | Reduce the page to the seven required navigation elements. Group the taxonomy leaves, restore the **Approx. opcodes** column with nearest-ten `~` values, keep the two short guidance sections to 3-4 sentences each, and remove the per-opcode, producer-count, migration-status, and wrapper-generation audits. |
| F-002 | minor | `## How AST nodes lower to IR`, lines 195-206 | The sentence claiming that the dependency pages mark seven unproduced resource opcodes and one unproduced value opcode is stale. The current pages identify nine resource opcodes and five value opcodes with no producer. | `docs/generated/design/ir-reference/resources-and-atomics.md:173-185`, `:243-244`, `:308`, and `:332` identify nine rows; `docs/generated/design/ir-reference/values.md:711-742` identifies `castToVoid`, `PtrCast`, `getAddr`, `alloca`, and `ReinterpretOptional`. | Delete this count as out of scope for the index. If retained, change the resource and value counts to nine and five and re-check them mechanically. |
| F-003 | minor | Final paragraph, lines 337-350 | The page says its manifest watches only `slang-ir-insts.lua` and the sibling-doc glob and that several source paths “should be added.” The current manifest already watches all seven additional source files, so the paragraph is both contradicted by the manifest and obsolete as a remediation note. | `docs/generated/design/_meta/manifest.yaml:554-563` lists `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h.lua`, `slang-lower-to-ir.cpp`, `slang-ast-expr.h`, the three meta-module files, and the sibling-doc glob. | Remove the obsolete paragraph. Do not replace it with another manifest-maintenance note, because the generation prompt defines this page as a reader-facing navigation hub. |

## No-issues notes
- Every cited Lua line and range resolves to the named entry at the recorded source commit.
- All ten family-page row totals and the reported placeholder and wrapper-marker counts match the current dependency pages.
- The mandatory front-matter fields are present and well formed; this self-watching index is subject to the `_common.md` moving-digest caveat.
