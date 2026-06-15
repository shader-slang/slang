---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:22+00:00
target_doc: ir-reference/misc.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/misc.md

## Summary
The page has valid front matter and its relative links resolve. I found two material issues: the catch-all coverage misses concrete capability-set opcodes, and the kernel-launch rows describe operand shapes that do not match the builder and emitter code that actually creates and consumes those instructions.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/misc.md`.
- Read `_common.md`, `ir-reference-misc.md`, the target document including front matter, dependency docs, and watched source files.
- Resolved the document's relative Markdown links and checked peer generated-doc links against the `docs/generated/design` tree.
- Verified required sections, table columns, front-matter keys, target source commit, and watched-path digest format.
- Spot-checked more than 10 source-alignment claims across `nop`, `Unrecognized`, `Expand`, `Each`, `MakeWitnessPack`, `PackBranch`, `IsType`, `sizeOf`, `alignOf`, `CastStorageToLogical`, `MakeStorageTypeLoweringConfig`, `liveRangeStart`, `getStringHash`, `DispatchKernel`, and `CudaKernelLaunch`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes` | The catch-all page omits the concrete `capabilityConjunction` and `capabilityDisjunction` opcodes. These are concrete stable opcodes under the top-level `CapabilitySet` group, are not listed on another assigned page, and fit the misc prompt's catch-all rule for unclaimed concrete Lua entries. | `source/slang/slang-ir-insts.lua:871` declares `CapabilitySet` with `capabilityConjunction` and `capabilityDisjunction`; `source/slang/slang-ir-insts-stable-names.lua:155-156` assigns stable names to both concrete opcodes. | Add a small capability-set sub-table with rows for `capabilityConjunction` and `capabilityDisjunction`, or explicitly move them to the owning family page and cross-link from misc. |
| F-002 | major | `### Kernel launch` and `### CudaKernelLaunch` | The `DispatchKernel` and `CudaKernelLaunch` rows/callout do not match the IR shape used by the builder and Torch emitter. The document lists `CudaKernelLaunch` as separate grid/block dimension operands and omits `DispatchKernel`'s trailing call arguments, but source creates `CudaKernelLaunch` with `baseFn, gridDim, blockDim, argsArray, cudaStream` and `DispatchKernel` with `baseFn, threadGroupSize, dispatchSize, args...`. | `source/slang/slang-ir.cpp:3618-3619` appends variadic call arguments to `DispatchKernel`; `source/slang/slang-ir.cpp:3630-3638` creates `CudaKernelLaunch` with five operands; `source/slang/slang-emit-torch.cpp:73-99` consumes operands 0-4 as function, grid dim, block dim, args, and stream. | Update the two kernel-launch rows and the `CudaKernelLaunch` notable callout to describe the actual builder/emitter operand shapes, and note the Lua declaration mismatch if the table still derives from `slang-ir-insts.lua`. |
