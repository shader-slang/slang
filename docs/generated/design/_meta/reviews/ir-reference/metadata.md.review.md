---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:07:16+00:00
target_doc: ir-reference/metadata.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 6ca22e11b1ae848bc68390906f1d20589efa4eb3e3366532aa60f8ccaecd4b6c
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 5
severity_breakdown:
  critical: 0
  major: 2
  minor: 3
  nit: 0
---

# Review report for ir-reference/metadata.md

## Summary
The 59-opcode catalog is broadly accurate, and every line-number citation resolves to the stated symbol at the recorded source commit. Two contract-wide omissions remain: the wrapper column does not identify the 38 hand-written wrappers, and nine AST-origin cells use the forbidden `(synthesized)` or bare-dash forms instead of exact producers or `no producer at HEAD`. The front-matter digest and manifest-coverage discussion also lag the current eight-file watched set.

## Items checked
- Reviewed the target, `_common.md`, the per-document prompt, all eight files from `regenerate.py show`, and both dependency documents.
- Verified all 40 line-number or line-range citation occurrences against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`; every cited line and range is accurate.
- Enumerated all 59 concrete rows against the Lua `Layout`, `Attr`, `Debug*`, `SPIRVAsm`, and `SPIRVAsmOperand` entries, including hierarchy, flags, wrappers, and operand shapes.
- Spot-checked more than 10 behavioral claims, including alignment ordering/defaults, stride derivation, tuple-field and debug-no-scope mismatches, semantic selection, debug operand shapes, layout producers, and inline-asm lowering/emission.
- Resolved all 38 Markdown link occurrences at the recorded commit and confirmed every generated peer target is present in the manifest.
- Ran identifier and whole-filename sweeps and recomputed the watched-path digest; the 12 prose filenames exist, while the recomputed digest differs as reported below.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source`, lines 39-51; all opcode tables | The page says the listed base/debug/asm structs are hand-written and “the rest are emitted by the FIDDLE template,” but many omitted concrete wrappers are also hand-written. The required per-row hand-written marker and prose count are absent throughout the catalog. | `source/slang/slang-ir-insts.h:1240-1819` hand-writes concrete layout/attribute wrappers such as `IRParameterGroupTypeLayout`, `IRVarLayout`, and `IRAlignedAttr`; `:2711-2819`, `:2854-2907`, and `:2959` add the debug, asm, and embedded wrappers. In total, 38 of the 59 table rows have hand-written wrappers. | Mark those 38 wrapper cells with a consistent footnote marker, state “38 hand-written wrappers” in the Source prose, and replace the incomplete list with an accurate explanation that only the other 21 wrappers are generated. |
| F-002 | major | `## Opcodes`, lines 137, 155-162, 190, and 193-194 | Nine AST-origin cells use a bare `—` or the retired `(synthesized)` catch-all, contrary to the mandatory producer contract. This hides the distinction between dormant opcodes and opcodes created by named functions or passes. | `docs/generated/design/_meta/prompts/_common.md:238-241` requires an exact producer or `no producer at HEAD`. Producers include `source/slang/slang-ir-specialize-function-call.cpp:618` (`nonuniform`), `source/slang/slang-ir.cpp:5555-5641` (`Aligned`/`MemoryScope`), `source/slang/slang-emit.cpp:970-1032` (`DebugBuildIdentifier`), and `source/slang/slang-compiler-tu.cpp:230` (`EmbeddedDownstreamIR`). | Replace `—` with `no producer at HEAD` for `tupleTypeLayout`, `tupleFieldLayout`, `caseLayout`, and `DebugInlinedVariable`; name the cited producer for `nonuniform`, `Aligned`, `MemoryScope`, `DebugBuildIdentifier`, and `EmbeddedDownstreamIR`. Also remove the Manifest-coverage suggestion to reduce origins to `(synthesized)`. |
| F-003 | minor | `## Source`, lines 63-67 | The sentence saying inlining and the debug-value-store pass “add the rest” of the `Debug*` opcodes is false and contradicts the later table: `DebugBuildIdentifier` is created by `linkAndOptimizeIR`, while `DebugInlinedVariable` has no producer at this commit. | `source/slang/slang-emit.cpp:970-1032` emits `DebugBuildIdentifier`; `source/slang/slang-ir.cpp:3665-3668` only defines the unused `emitDebugInlinedVariable` builder. | Enumerate the pass-produced debug opcodes explicitly, then state separately that `linkAndOptimizeIR` creates `DebugBuildIdentifier` and `DebugInlinedVariable` has no producer at HEAD. |
| F-004 | minor | Front matter, lines 1-8 | `target_doc_watched_paths_digest` is stale for the current manifest entry. The report input resolves eight watched files, but recomputation yields `af489c60cd80bc947126003946e25cbb5b12ef066e89435deb8da854a7fe280d`, not the recorded `6ca22e…`. | `docs/generated/design/_meta/manifest.yaml:715-728` includes the eight watched paths; `python3 docs/generated/design/_meta/regenerate.py digest ir-reference/metadata.md` returns the differing digest. | Regenerate or update the target front matter so `watched_paths_digest` records `af489c60cd80bc947126003946e25cbb5b12ef066e89435deb8da854a7fe280d`. |
| F-005 | minor | `## Manifest coverage`, lines 439-451 | The section still lists `slang-emit.cpp` and `slang-emit-spirv.cpp` as outside `watched_paths` and says all six named paths should be added, but both files are already watched. Only the four pass files remain outside the current entry. | `docs/generated/design/_meta/manifest.yaml:720-728` includes `source/slang/slang-emit.cpp` and `source/slang/slang-emit-spirv.cpp`. | Remove those two files from the unwatched-path list, change “six paths” to “four paths,” and retain only the four actual pass-file follow-ups. |
