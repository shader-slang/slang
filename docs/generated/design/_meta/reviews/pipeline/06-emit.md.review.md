---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T19:58:11Z
target_doc: pipeline/06-emit.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 6d61a0c555cb971c2572a94ab5ecb069df4bd3740e86f2deefeb31ffb4f076aa
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 0
---

# Review report for pipeline/06-emit.md

## Summary

The document covers the required structure and backend inventory, and its line-number citations match the source. Three factual corrections remain. Most importantly, the CUDA function-qualifier description omits explicit exceptions that change whether emitted functions execute on the host or device and whether they are kernels.

## Items checked

- Reviewed against the CURRENT working tree; `git diff` confirmed that `source/` equals the document’s recorded source commit. Confirmed HEAD matches the supplied report commit. No files were written.
- Read `_review.md`, `_common.md`, the document’s generation prompt, and the resolved watched paths from `regenerate.py show pipeline/06-emit.md`.
- Checked all 21 source-location citations, including the 125–128 range, across 10 source files. No line-number drift found.
- Spot-checked more than 20 behavioral claims: emitter selection, line-directive defaults, legalization options, output assembly, artifact metadata, SPIR-V legalization and downstream gates, C++ wrappers, CUDA qualifiers, LLVM outputs, VM stage filtering and diagnostics, Slang declaration stub, emitter inheritance, Metal extension tracking, WGSL switch handling, precedence, prelude registration, and dependency-file output.
- Resolved all 94 file-link occurrences to 59 distinct existing targets and checked both internal heading links. All three referenced generated pages are manifest entries. Read dependency-document excerpts only for cross-reference checks.
- Swept 144 identifier candidates and 53 filename candidates; unmatched items were illustrative names, test directives, metadata vocabulary, or the external NVAPI header.
- Checked required sections, all 11 backend subsections, front-matter keys and formats, style, and Liquid safety. The document is 27,179 bytes, below its 32,768-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### CUDA`, lines 258–260 | The statement that the preamble qualifies an entry point `extern "C" __global__` and every ordinary function `__device__` omits explicit alternative execution qualifiers. Callable entry points use `__device__`; non-entry-point functions carrying CUDA kernel or host decorations use `__global__` or `__host__`. | `source/slang/slang-emit-cuda.cpp:479` checks `Stage::Callable` and emits `__device__`. Lines 491–497 handle `IRCudaKernelDecoration` and `IRCudaHostDecoration` before the default device branch. | Describe all branches: callable entry points are `extern "C" __device__`, other entry points are `extern "C" __global__`; non-entry-point kernel/host decorations select their respective qualifiers, otherwise the function uses `__device__`. |
| F-002 | minor | `## Adding a new backend`, lines 488–492 | The VM example incorrectly names `emitVMByteCodeForEntryPoints` as a function that calls `linkAndOptimizeIR` and is invoked from `_emitEntryPoints`. That orchestration belongs to `emitHostVMCode`. | `source/slang/slang-code-gen.cpp:1202` calls `emitHostVMCode`. `source/slang/slang-emit.cpp:3819` defines that wrapper, which links IR before calling the bytecode emitter. `source/slang/slang-emit-vm.cpp:1411` accepts an existing `LinkedIR&`. | Replace `emitVMByteCodeForEntryPoints` in this checklist with `emitHostVMCode`; identify the former as its lower-level bytecode emitter if retaining additional detail. |
| F-003 | minor | `### Shared C-like base`, lines 329–332 | The examples of the overridable “virtual interface” name non-virtual entry wrappers: `emitDeclarator`, `emitType`, and `emitOperand`. Subclasses override their implementation hooks. | `source/slang/slang-emit-c-like.h:291` declares the non-virtual declarator wrapper; line 292 declares virtual `emitDeclaratorImpl`. Lines 661 and 667 declare virtual `emitTypeImpl` and `emitOperandImpl`; the operand wrapper is at line 371. | Name `emitDeclaratorImpl`, `emitTypeImpl`, and `emitOperandImpl` as the virtual override points, distinguishing them from the public wrappers. |