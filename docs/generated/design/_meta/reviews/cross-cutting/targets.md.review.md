---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:23+00:00
target_doc: cross-cutting/targets.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 720cbadffe0ddbcfd07c03b208f3f7cbad55f384b2abb3ca09da30eb7d155f95
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 2
  major: 2
  minor: 1
  nit: 0
---

# Review report for cross-cutting/targets.md

## Summary

The page has the required structure, valid links, and mostly accurate capability definitions, but it contains five source-alignment issues. Most importantly, its `-target spirv -profile glsl_450` example describes an impossible mixed-target capability set, and its new-target checklist incorrectly instructs backend authors to include an embedded prelude as a header.

## Items checked

- Read the target page, `_common.md`, the per-document prompt, both dependency pages, and all 40 resolved watched files reported by `regenerate.py show`.
- Verified source against the recorded commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, which was also `HEAD` at review time.
- Spot-checked more than 10 claims, including the complete public target enum, `SourceLanguage`, capability definition forms and keyholes, latest-version aliases/accessors, `CapabilitySet`, `Profile`, `TargetRequest::getTargetCaps`, explicit-capability diagnostics, target/stage specialization, emitter dispatch, prelude insertion, and Slang re-emission.
- Ran the generated-doc structural lint successfully, checked all relative link targets and peer-doc manifest entries, and confirmed the 21,849-byte page is under its 24,576-byte cap.
- Verified every body line-number citation: the page contains none.

## Findings

| ID    | Severity | Location                                        | Description                                                                                                                                                                                                                                                                                                                                                 | Evidence                                                                                                                                                                                                                                                                                            | Recommendation                                                                                                                                                                                                           |
| ----- | -------- | ----------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F-001 | critical | `## Profiles`, lines 258-260                    | The example says `-target spirv -profile glsl_450` produces a `CapabilitySet` containing both `glsl_450` atoms and the `spirv` target key. Those target atoms are incompatible, and the implementation never constructs that combination: direct SPIR-V keeps only SPIR-V version/extension atoms from the profile, while the via-GLSL path selects `glsl`. | `source/slang/slang-target.cpp:99-150` implements the two SPIR-V paths; `source/slang/slang-target.cpp:209-213` joins profile capabilities only when compatible. `source/slang/slang-capabilities.capdef:162-180` shows SPIR-V versions derive from `spirv` while GLSL versions derive from `glsl`. | Replace the example with separate direct-SPIR-V and SPIR-V-via-GLSL cases, stating which target atom and compatible version family each path retains.                                                                    |
| F-002 | major    | `## Profiles`, lines 239-256                    | The section says profiles “bundle a target choice” and that `Profile` carries a target-language hint. `Profile` actually encodes a stage and version, derives its family from the version, and has no target-format or language-hint field; the format/profile pairing belongs to `TargetDesc`/`TargetRequest`.                                             | `source/slang/slang-profile.h:68-115` lists every `Profile` field and accessor. `include/slang.h:4353-4367` places `format` and `profile` together in `TargetDesc`.                                                                                                                                 | Define `Profile` as stage plus version (with family derived from the version), remove the language-hint claim, and attribute the format/profile pairing to `TargetDesc` or `TargetRequest`.                              |
| F-003 | major    | `## Targets` table, line 45                     | The “Slang round-trip” row claims the internal path re-emits Slang source. At the recorded commit the only such helper is a stub that leaves the output unchanged, and there is no `CodeGenTarget::Slang`.                                                                                                                                                  | `source/slang/slang-emit-slang.cpp:6-14` ignores all inputs and returns success without writing output. `source/slang/slang-target.h:24-61` contains no Slang code-generation target.                                                                                                               | Remove this from the supported-target table, or label it explicitly as an unimplemented internal stub that currently produces no source.                                                                                 |
| F-004 | critical | `## Adding a new target`, step 4, lines 367-369 | The checklist instructs a backend author to emit a `#include` for a prelude header. Existing language preludes are embedded strings registered on the global session and written directly into generated source; following the instruction would create an undeployed-header dependency instead of integrating the prelude.                                 | `source/slang/slang-global-session.cpp:125-128` registers the CUDA, C++, and HLSL prelude strings. `source/slang/slang-emit.cpp:2937-2952` writes the selected prelude string directly through `SourceWriter`.                                                                                      | Say to embed/register the prelude for the new `SourceLanguage` and let `emitEntryPointsSourceFromIR` write it. Mention `#include` only for a separate runtime header that is deliberately shipped with generated output. |
| F-005 | minor    | `## Targets` table, line 35                     | The GLSL row calls `SLANG_GLSL_VULKAN_*` “aliases.” The API instead contains two specifically named deprecated-and-removed enum slots; they are not aliases and the wildcard spelling is not an identifier readers can search for.                                                                                                                          | `include/slang.h:687-689` declares `SLANG_GLSL_VULKAN_DEPRECATED` and `SLANG_GLSL_VULKAN_ONE_DESC_DEPRECATED` and labels them deprecated/removed.                                                                                                                                                   | Name both enumerators explicitly and describe them as retained deprecated enum values, not aliases.                                                                                                                      |

## No-issues notes

- All mandatory front-matter keys are present, and the recorded digest is a valid SHA-256 value.
- The capability keyhole arithmetic, latest-version aliases/accessors, auto-doc groups, and explicit `-capability` conflict diagnostic matched source.
- The required target, capability, profile, IR-effects, and new-target sections are all present.
