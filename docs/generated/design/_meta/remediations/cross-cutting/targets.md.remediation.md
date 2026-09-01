---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:35:00Z
target_doc: cross-cutting/targets.md
review_report: ../../reviews/cross-cutting/targets.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/targets.md

## Summary

All five findings were verified against source at
`53b76e6d3009b8e6434d41573524c7ce5c499d23` and all five were fixed. The two
critical items — the impossible `-target spirv -profile glsl_450` capability
set and the prelude-as-`#include` instruction in the new-target checklist —
were both confirmed accurate and rewritten. The two major items (the `Profile`
field description and the Slang round-trip table row) and the one minor item
(the GLSL deprecated-enumerator wording) were likewise confirmed and corrected.
Nothing was rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `TargetRequest::getTargetCaps` in `source/slang/slang-target.cpp` filters profile atoms through `isSpirvVersionAtom` / `isSpirvExtensionAtom` on the direct path and selects `CapabilityName::glsl` on the via-GLSL path; the trailing `join` is guarded by `atLeastOneSetImpliedInOther`, so a GLSL profile contributes nothing to a `spirv` target set. | `## Profiles`: replaced the one-sentence mixed-target example with a paragraph describing the direct-SPIR-V and via-GLSL paths separately and the `spirv_1_5` default. |
| F-002 | fixed | Confirmed: `Profile` in `source/slang/slang-profile.h` packs only stage and version into `raw`, derives `ProfileFamily` via `getFamily()`, and has no format or language field; `format` and `profile` sit together in `TargetDesc` in `include/slang.h`. | `## Profiles`: reworded the lead sentence and replaced the `Family`/`Version` and language-hint bullets with one bullet naming version-derived family and attributing format pairing to `TargetDesc`/`TargetRequest`. |
| F-003 | fixed | Confirmed: `source/slang/slang-emit-slang.cpp` contains only `emitSlangDeclarationsForEntryPoints`, which `SLANG_UNUSED`s all three parameters and returns `SLANG_OK` without writing output. | `## Targets` table: relabelled the Slang round-trip row as an unimplemented stub with no `CodeGenTarget` value. |
| F-004 | fixed | Confirmed: the `Session` constructor in `source/slang/slang-global-session.cpp` registers embedded CUDA, C++, and HLSL prelude strings into `m_languagePreludes`, and `emitEntryPointsSourceFromIR` in `source/slang/slang-emit.cpp` writes the selected string straight through `sourceWriter.emit`. No emitter emits a prelude `#include`. | `## Adding a new target` step 4: replaced the `#include` instruction with register-the-embedded-prelude-for-the-`SourceLanguage` guidance, retaining `#include` only for a separately shipped runtime header. |
| F-005 | fixed | Confirmed: `include/slang.h` declares `SLANG_GLSL_VULKAN_DEPRECATED = 3` and `SLANG_GLSL_VULKAN_ONE_DESC_DEPRECATED = 4`, each commented "deprecated and removed"; neither is an alias of `SLANG_GLSL`. | `## Targets` table, GLSL row: named both enumerators explicitly and described them as retained deprecated values rather than aliases. |

## Notes for the operator

- The page is now 23,646 bytes against a 24,576-byte cap, leaving under 1 KB of
  headroom. A future cycle that needs to add material here should request a cap
  increase rather than trim these corrections.
- The F-004 fix cites `source/slang/slang-global-session.cpp`, which is not in
  this page's `watched_paths`. The page already cited the similarly unwatched
  `source/slang/slang-emit.cpp`, `source/slang/slang-target.cpp`, and
  `include/slang.h` before this cycle. Consider adding
  `source/slang/slang-target.{h,cpp}`, `source/slang/slang-emit.cpp`, and
  `source/slang/slang-global-session.cpp` to the manifest entry so the page's
  target-dispatch, capability-derivation, and prelude claims are covered by the
  digest.
