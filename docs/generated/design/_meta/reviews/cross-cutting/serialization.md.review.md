---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:58+00:00
target_doc: cross-cutting/serialization.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: f1411119c2984cf871fda3e87109caf5abb8a34836f05a251561d7792998a19a
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: fail
finding_count: 4
severity_breakdown:
  critical: 1
  major: 2
  minor: 1
  nit: 0
---

# Review report for cross-cutting/serialization.md

## Summary

Most source-behavior explanations are accurate and all links resolve, but the page has four findings. Most importantly, it incorrectly tells readers that `-load-repro` is deprecated even though the repository guidance says to use it for specialized repro handling; its watched-path digest and manifest-coverage discussion are also stale.

## Items checked

- Reviewed the target, `_common.md`, the per-document prompt, the `architecture/overview.md` dependency, and all 21 files resolved by `regenerate.py show`.
- Verified 18 source-backed claims at `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including generic read/write dispatch, shared-pointer handling, enum generation, Fossil validation and relative pointers, RIFF chunks, flat-IR tables and validation, unknown-op handling, source-location reconstruction, stable opcode names, and serialization version 1.
- Verified all relative links (25 unique targets) at the recorded source commit; all resolve. The body contains zero line-number citations, so every such citation was verified vacuously.
- Ran identifier and file-name sweeps and confirmed the document is 18,827 bytes, below its 24,576-byte cap.
- Recomputed the watched-path digest after confirming the resolved watched source files match the recorded source commit byte-for-byte.

## Findings

| ID    | Severity | Location                                                   | Description                                                                                                                                                                                                                                                                                                                                                                                                    | Evidence                                                                                                                                                                                                    | Recommendation                                                                                                                                                                                                                                                                   |
| ----- | -------- | ---------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | Front matter, line 6                                       | The recorded digest is `f141...a19a`, but the current resolved watched set produces `ec2dbba845c65be8a0b4a6e9917fe98983520d876c47130c349d660c048c7927`. The metadata therefore does not identify the watched inputs supplied for this review.                                                                                                                                                                  | `docs/generated/design/_meta/manifest.yaml:925-950` defines the resolved watched set; `regenerate.py digest cross-cutting/serialization.md` returns `ec2d...927`.                                           | Regenerate the page against the current resolved watched set and record the recomputed digest.                                                                                                                                                                                   |
| F-002 | major    | `## Manifest coverage`, lines 368-388                      | The section says four cited files are outside `watched_paths` and recommends adding them, but every named file is already watched. It also overlooks the now-watched stable-name implementation `.cpp`, so the freshness guidance is wholly stale.                                                                                                                                                             | `docs/generated/design/_meta/manifest.yaml:940-950` watches `slang-serialize-types.{h,cpp}` and all three `slang-ir-insts-stable-names` files.                                                              | Remove the add-path recommendation and replace the section with an accurate statement that the cited serialization types, stable-name implementation, and core RIFF files are covered.                                                                                           |
| F-003 | minor    | `## Versioning and backwards compatibility`, lines 285-296 | The stable-name mapping discussion links the declaration header and generator input but never mentions `slang-ir-insts-stable-names.cpp`, despite the per-document checklist requiring every relevant watched file to be mentioned. That `.cpp` owns both mapping tables and the two conversion functions.                                                                                                     | `source/slang/slang-ir-insts-stable-names.cpp:6-40,72-90`; the file is watched at `docs/generated/design/_meta/manifest.yaml:949`.                                                                          | Link `slang-ir-insts-stable-names.cpp` in this paragraph and identify it as the implementation of the opcode-to-stable-name and stable-name-to-opcode mappings.                                                                                                                  |
| F-004 | critical | `## Round-trip and repro files`, lines 313-320             | The page groups `-load-repro` with `-dump-repro` as deprecated and says it should not be relied on. Repository guidance discourages `-dump-repro`, but explicitly says `-load-repro` and `-extract-repro` are specialized tools to use when working on repro handling. The per-document prompt repeats the stale premise, but the resulting reader instruction still contradicts the source of truth it cites. | `CLAUDE.md:324-333` places only `-dump-repro` in the “DO NOT USE” list and directs repro work to `-load-repro` / `-extract-repro`; `source/slang/slang-options.cpp:1071-1092` still registers both options. | After correcting the stale premise in the per-document prompt out of band, state that `-dump-repro` is discouraged while `-load-repro` and `-extract-repro` remain specialized repro-handling tools; keep implementation details out of scope because their files are unwatched. |
