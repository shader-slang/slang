---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:12+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 33e2de2e8f7d94757701c9d2589b0e90ad221fa2f6afba49347b2d79a916ab4c
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 2
  minor: 1
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary

The page has the required target-pipeline shape and most of the `linkAndOptimizeIR` SPIR-V ordering is aligned with the source, but several high-impact details are wrong. The most important issue is the `simplifyIRForSpirvLegalization` loop description: the document promises active 8 and 16 iteration bounds, while the source counters are never incremented at the reviewed commit. The downstream validation and assembly-target scope text also needs correction before remediation marks this page reliable.

## Items checked

- Ran `regenerate.py show target-pipelines/spirv.md` and verified the prompt path, watched paths, dependency list, and target document front-matter against the manifest output.
- Read `_common.md`, `_review.md`, `target-pipelines-spirv.md`, the target document, and the five `depends_on` generated docs.
- Traced `linkAndOptimizeIR` from `source/slang/slang-emit.cpp` lines 895-2521 against the Phase A-C tables, including SPIR-V-only gates and sibling-target branches that should be filtered out.
- Checked Phase D against `createArtifactFromIR`, `emitSPIRVForEntryPointsDirectly`, `emitSPIRVFromIR`, and `legalizeIRForSPIRV` in the watched source files.
- Verified the loop, downstream validation/linking, `SPIRVAssembly`, and `shouldEmitSPIRVDirectly` claims with targeted source searches and line-range reads.
- Checked required section order, table columns, front-matter keys, and relative-link style for the generated document.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase D` diagram and `## Loops in the pipeline`, lines 643-684 and 829-848 | The document claims `simplifyIRForSpirvLegalization` is bounded by `i<8` and `j<16`, says the loops terminate when those bounds are reached, and gives a worst-case count of `8 * 16 = 128` per-function sub-passes. At this source commit, the loop conditions mention `iterationCounter` and `funcIterationCount`, but neither counter is incremented, so the documented bound behavior is not what the source implements. | `source/slang/slang-ir-spirv-legalize.cpp:2881` declares `kMaxIterations = 8`, `source/slang/slang-ir-spirv-legalize.cpp:2883` declares `iterationCounter = 0`, `source/slang/slang-ir-spirv-legalize.cpp:2885` tests it, and `source/slang/slang-ir-spirv-legalize.cpp:2901`-`source/slang/slang-ir-spirv-legalize.cpp:2902` does the same for `funcIterationCount`, with no increment in the loop bodies through `source/slang/slang-ir-spirv-legalize.cpp:2915`. | Rewrite the loop diagram and loop prose to describe the source as it exists: the constants and guard expressions are present, but the counters are not advanced, so the document must not promise bound-based termination or a finite worst-case pass count unless the source is fixed first and the doc regenerated. |
| F-002 | major | Phase D table row 18 and `### Downstream spirv-link / spirv-val / spirv-opt chain`, lines 725 and 955-958 | The document says `spirv-val` is enabled by `-validate-spirv` or a non-empty `SLANG_RUN_SPIRV_VALIDATION` environment variable. The reviewed source has no `-validate-spirv` option, requires the environment variable to be exactly `"1"`, and also disables validation for incomplete libraries. | `source/slang/slang-emit.cpp:3045`-`source/slang/slang-emit.cpp:3067` returns false when `SkipSPIRVValidation` or `IncompleteLibrary` is set and returns true only when `SLANG_RUN_SPIRV_VALIDATION == "1"`. `source/slang/slang-options.cpp:684`-`source/slang/slang-options.cpp:685` defines `-skip-spirv-validation`; no matching `-validate-spirv` option exists in `source/slang/slang-options.cpp`. | Replace the validation trigger text with the actual gate: validation runs only when a downstream compiler is available, `SkipSPIRVValidation` and `IncompleteLibrary` are false, and `SLANG_RUN_SPIRV_VALIDATION` equals `"1"`. Remove all `-validate-spirv` and "non-empty env var" claims. |
| F-003 | major | Intro, lines 12-16 | The page says the direct-emit pipeline's corresponding `CodeGenTarget` values are `CodeGenTarget::SPIRV` and `CodeGenTarget::SPIRVAssembly`. In the public emit dispatcher, `SPIRVAssembly` first creates an intermediate `CodeGenContext` for `CodeGenTarget::SPIRV` and then disassembles the resulting artifact; the direct SPIR-V emit function is invoked from the `CodeGenTarget::SPIRV` case. | `source/slang/slang-code-gen.cpp:1047`-`source/slang/slang-code-gen.cpp:1057` maps `CodeGenTarget::SPIRVAssembly` to `CodeGenTarget::SPIRV`; `source/slang/slang-code-gen.cpp:1089`-`source/slang/slang-code-gen.cpp:1107` compiles the intermediate artifact and disassembles it; `source/slang/slang-code-gen.cpp:1154`-`source/slang/slang-code-gen.cpp:1158` calls `emitSPIRVForEntryPointsDirectly` only for `CodeGenTarget::SPIRV`. | Narrow the intro to `CodeGenTarget::SPIRV` with `shouldEmitSPIRVDirectly() == true`, then add a note that `SPIRVAssembly` reuses this pipeline indirectly by compiling the intermediate SPIR-V target before downstream disassembly. Adjust table notes that imply the public pipeline runs `linkAndOptimizeIR` with `target == SPIRVAssembly`. |
| F-004 | minor | Phase D table rows 16 and 19, lines 723 and 726 | The Phase D table cites disabled `optimizeSPIRV` as if it lives in `slang-emit-spirv.cpp`, and it says the downstream `compile` step is "always invoked". The disabled `#if 0` block is in `slang-emit.cpp`, and the active downstream `compiler->compile` call is guarded by successful loading of a `PassThroughMode::SpirvOpt` downstream compiler. | `source/slang/slang-emit.cpp:3092`-`source/slang/slang-emit.cpp:3099` contains the disabled `optimizeSPIRV` call. `source/slang/slang-emit.cpp:3103`-`source/slang/slang-emit.cpp:3107` loads `PassThroughMode::SpirvOpt` and wraps the downstream chain in `if (compiler)`, with `compiler->compile` at `source/slang/slang-emit.cpp:3209`-`source/slang/slang-emit.cpp:3210`. No `optimizeSPIRV` symbol appears in `source/slang/slang-emit-spirv.cpp`. | Move the disabled `optimizeSPIRV` citation to `slang-emit.cpp` and avoid presenting it as an active downstream node. Change the downstream `compile` gate to `compiler != nullptr` plus the optimization-level options, rather than "always invoked". |

## No-issues notes

- The required target-pipeline sections are present in the expected order, including Source, high-level diagram, four phase sections, Conditional gates, Loops, Notable passes, and See also.
- The Phase A-C tables use the required columns and generally preserve the reachable `SLANG_PASS` order from `linkAndOptimizeIR` for the direct SPIR-V path.
- The direct emit and legalization files named by the Source section are part of the manifest's resolved watched files.
- The `legalizeEntryPointsForGLSL`, second `specializeFuncsForBufferLoadArgs`, and SPIR-V-specific `eliminatePhis` option callouts are supported by the checked source ranges.
