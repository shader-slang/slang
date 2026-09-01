---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:42+00:00
target_doc: ir-reference/differentiation.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: fail
finding_count: 6
severity_breakdown:
  critical: 1
  major: 1
  minor: 2
  nit: 2
---

# Review report for ir-reference/differentiation.md

## Summary

The opcode inventory, wrappers, producer attributions, citations, and cross-references are generally accurate. The most important error is the `TrivialForwardDifferentiate` summary: source explicitly zeros output differentials rather than leaving them alone. The recorded watched-path digest also predates the current resolved watched set.

## Items checked

- Reviewed the target, both generation prompts, all four dependency documents, and the 28 resolved watched files reported by `regenerate.py show`.
- Verified all 66 explicit line/range citation expressions against source at `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Checked all 39 opcode rows against the Lua declarations, generated and hand-written wrappers, lowering/checking producers, translation switch, and cited autodiff passes.
- Resolved all 67 relative link occurrences, checked all 13 linked-anchor occurrences, and confirmed the generated peer targets are present in the manifest.
- Ran identifier and whole-filename sweeps, including the intentionally absent `slang-ir-autodiff-transcribe.cpp` and obsolete context spellings.
- Recomputed the watched-path digest, checked the 38,691-byte page against its 65,536-byte cap, and validated the mandatory front-matter fields.

## Findings

| ID    | Severity | Location                                       | Description                                                                                                                                                                                                                                                                                     | Evidence                                                                                                                                                                                                                                                     | Recommendation                                                                                                                                                     |
| ----- | -------- | ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F-001 | critical | `TrivialForwardDifferentiate` row, line 199    | The summary says the derivative “leaves differentials alone,” but the implementation deliberately discards incoming tangents and forces output differentials to zero. This reverses the opcode's essential derivative behavior.                                                                 | `source/slang/slang-ir-autodiff-fwd.cpp:191-205` documents and implements `zeroDifferentials`; `source/slang/slang-ir-autodiff-fwd.cpp:226-249` calls the primal and pairs its result with a zero differential.                                              | Replace the summary with wording such as “Runs the primal and returns zero output differentials, ignoring incoming tangents.”                                      |
| F-002 | minor    | `## Source`, lines 106-116                     | “Nothing in this family survives to emit” is too strong. `IRBuiltinRequirementKey` may survive as an unreferenced global until C-like emission, where the emitter explicitly skips it. It produces no target code, but it can reach the emitter.                                                | `source/slang/slang-emit-c-like.cpp:5287-5292` says the hoistable key may survive after specialization and skips it explicitly.                                                                                                                              | Narrow the claim to say that these opcodes produce no target code, and mention that `IRBuiltinRequirementKey` can survive to emission as metadata that is ignored. |
| F-003 | major    | Front matter, lines 1-7                        | `watched_paths_digest` is stale for the current manifest entry. The recorded value is `64be22...`, while `regenerate.py digest ir-reference/differentiation.md` returns `57ecc66...` after the watched set was expanded to the autodiff, translation, DCE, and checker files used by this page. | `docs/generated/design/_meta/manifest.yaml:677-692` defines the current watched set; `python3 docs/generated/design/_meta/regenerate.py digest ir-reference/differentiation.md` returned `57ecc66ac078185e6feb45fd601a286e532e930b62c724711b6b6be62d2fd570`. | Regenerate the page against the current resolved watched set and record the resulting digest in front matter.                                                      |
| F-004 | minor    | `DiffTypeInfo` row, line 266                   | The operand cell says `† (none declared)`, but `DiffTypeInfo` is a nullary opcode and the family contract requires `—` for nullary rows.                                                                                                                                                        | `source/slang/slang-ir-insts.lua:1123-1127` declares no operands or `min_operands`; `docs/generated/design/_meta/prompts/_common.md:238` requires `—` for nullary opcodes.                                                                                   | Change the Operands cell to `—`; no dagger explanation is needed for a genuine nullary opcode.                                                                     |
| F-005 | nit      | Introductory paragraphs, lines 12-23           | The first paragraph explains the scope, but the intended reader appears only in a separate second paragraph. The universal contract requires both in the first paragraph.                                                                                                                       | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state what the document covers and who it is for.                                                                                                                | Merge the intended-reader sentence into the opening paragraph.                                                                                                     |
| F-006 | nit      | `BackwardDifferentiate` callout, lines 408-410 | The statement that the attributes are declared in `diff.meta.slang` uses a bare backticked filename instead of a workspace-relative Markdown link.                                                                                                                                              | `docs/generated/design/_meta/prompts/_common.md:43-45` requires source-file citations to be Markdown links.                                                                                                                                                  | Link this occurrence to `../../../../source/slang/diff.meta.slang`, matching the existing link near the top of the page.                                           |
