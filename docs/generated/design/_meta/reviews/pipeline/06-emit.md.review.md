---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:05+00:00
target_doc: pipeline/06-emit.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: ea24fee55e05b1bf9f8539d729e088b731324bd838dcfea53da87cfcb44cd959
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for pipeline/06-emit.md

## Summary
The page is structurally complete and most checked claims match the watched emit sources, but it overgeneralizes the textual-source dispatcher as if it were the registration point for every backend. The most important issue is the new-backend checklist: a reader adding a non-textual backend would be directed to wire it through `CodeGenContext::emitEntryPointsSourceFromIR`, while the watched source shows SPIR-V, LLVM, and VM bytecode use separate direct emit functions.

## Items checked
- Ran `regenerate.py show pipeline/06-emit.md` and reviewed the target document, the common contract, its per-document prompt, and the dependency document `docs/generated/design/pipeline/05-ir-passes.md`.
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved 75 relative markdown links from the document body and verified they point at workspace files or generated peer documents.
- Reviewed the page against its per-document prompt and common generated-doc contract.
- Spot-checked source-backed claims against the watched files, including `linkAndOptimizeIR`, `CodeGenContext::emitEntryPointsSourceFromIR`, `CLikeSourceEmitter::getSourceLanguage`, `SourceWriter`, `LineDirectiveMode`, `advanceToSourceLocation`, `emitDeclarator`, `emitType`, `emitOperand`, `EmitOpInfo`, `emitSPIRVForEntryPointsDirectly`, `emitLLVMForEntryPoints`, `emitVMByteCodeForEntryPoints`, `writeDependencyFile`, `HLSLSourceEmitter`, `GLSLSourceEmitter`, `MetalSourceEmitter`, `WGSLSourceEmitter`, `CPPSourceEmitter`, `CUDASourceEmitter`, and `TorchCppSourceEmitter`.
- Reviewed the two prose line-number claims in the page (`linkAndOptimizeIR` at line 896 and `emitEntryPointsSourceFromIR` at line 2540).

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Adding a new backend`, lines 230-233 | The checklist says to register a new backend in `emitEntryPointsSourceFromIR`, but that function only constructs C-like textual emitters plus the PyTorch C++ binding. The watched sources show non-textual outputs are emitted through separate functions, so this instruction would send a reader adding a SPIR-V/LLVM/VM-style backend to the wrong registration point. | `source/slang/slang-emit.cpp` lines 2613-2655 select `CPPSourceEmitter`, `GLSLSourceEmitter`, `HLSLSourceEmitter`, `CUDASourceEmitter`, `MetalSourceEmitter`, `WGSLSourceEmitter`, or `TorchCppSourceEmitter`; `source/slang/slang-emit.cpp` lines 3251-3259 define `emitSPIRVForEntryPointsDirectly`, and lines 3338-3354 define `emitLLVMForEntryPoints`; `source/slang/slang-emit-vm.cpp` lines 1191-1193 define `emitVMByteCodeForEntryPoints`. | Qualify the checklist step as the textual-backend path, and add a short note that direct/non-textual backends follow the separate emit functions used by SPIR-V, LLVM, and VM bytecode rather than `emitEntryPointsSourceFromIR`. |
| F-002 | minor | `## Emit dispatcher`, lines 50-52 | The statement that the `#include`s at the top of `slang-emit.cpp` are the authoritative list of linked backends is too broad. Direct SPIR-V is a documented backend in this page and is present in the watched files, but `slang-emit.cpp` does not include a `slang-emit-spirv.h`; it forward-declares `emitSPIRVFromIR` and relies on the implementation in `slang-emit-spirv.cpp`. | `source/slang/slang-emit.cpp` lines 14-25 include several emit headers but no SPIR-V emitter header; `source/slang/slang-emit.cpp` lines 2787-2791 forward-declare `emitSPIRVFromIR`; `source/slang/slang-emit-spirv.cpp` lines 11803-11805 define `emitSPIRVFromIR`. | Replace the sentence with a narrower claim, such as saying the includes identify the header-backed emit helpers used by this file, while direct SPIR-V is wired by the forward declaration and `slang-emit-spirv.cpp` implementation. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The required backend subsections from the per-doc prompt are all present.
