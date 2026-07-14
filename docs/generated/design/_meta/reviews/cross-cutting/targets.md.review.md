---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:29:34+00:00
target_doc: cross-cutting/targets.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 720cbadffe0ddbcfd07c03b208f3f7cbad55f384b2abb3ca09da30eb7d155f95
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for cross-cutting/targets.md

## Summary
The page satisfies the required structure and most sampled capability/profile claims match the checked sources. The target table has one substantive backend-mapping issue: it groups object-code and host-callable CPU targets under the C++ source emitter even though the checked emit path routes those `CodeGenTarget` cases through the LLVM emitter when using LLVM-backed CPU output.

## Items checked
- Ran `regenerate.py show cross-cutting/targets.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-capabilities.capdef`, `source/slang/slang-capability.cpp`, `source/slang/slang-capability.h`, `source/slang/slang-emit-base.cpp`, `source/slang/slang-emit-base.h`, plus 35 more).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Reviewed the page against its per-document prompt and common generated-doc contract, including the required targets, capability system, profiles, IR-target effects, and new-target checklist sections.
- Checked relative markdown links in the document body for workspace-relative shape and existing workspace targets.
- Spot-checked more than 10 source-backed claims and named symbols against the watched and directly cited files, including `SourceLanguage`, `Profile`, `ProfileVersion`, `CapabilitySet`, `join`, `abstract target`, `abstract stage`, `glsl`, `spirv`, `raytracing`, `abort`, `rayquery_sphere_nv`, `rayquery_lss_nv`, `subgroup_workgroup_index`, `emitEntryPointsSourceFromIR`, `CPPSourceEmitter`, `GLSLSourceEmitter`, `HLSLSourceEmitter`, `CUDASourceEmitter`, `MetalSourceEmitter`, `WGSLSourceEmitter`, and `TorchCppSourceEmitter`.
- Confirmed the page has no markdown source line-anchor citations requiring exhaustive line-anchor verification.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Targets` table, lines 32-45 | The `C++ host` row groups `SLANG_HOST_HOST_CALLABLE`, `SLANG_SHADER_HOST_CALLABLE`, `SLANG_HOST_OBJECT_CODE`, and `SLANG_OBJECT_CODE` under `slang-emit-cpp.cpp` and describes binaries as downstream-compiled. The checked target mapping and emit path show the object-code and host-callable `CodeGenTarget` cases are handled by `emitLLVMForEntryPoints`/`slang-emit-llvm.cpp` when using the LLVM-backed CPU path, so this row sends readers to the wrong backend for those formats. | `source/slang/slang-target.h:49-50` maps `SLANG_OBJECT_CODE` and `SLANG_HOST_HOST_CALLABLE` to `CodeGenTarget::ShaderObjectCode` and `CodeGenTarget::HostHostCallable`; `source/slang/slang-target.h:58-60` maps `SLANG_HOST_OBJECT_CODE`, `SLANG_HOST_LLVM_IR`, and `SLANG_SHADER_LLVM_IR`. `source/slang/slang-emit.cpp:3338-3388` implements `emitLLVMForEntryPoints`, with object-code cases calling `emitLLVMObjectFromIR` and host-callable cases calling `emitLLVMJITFromIR`. | Split the CPU/host row: keep C/C++ source/header and downstream C++ executable/shared-library flows with `slang-emit-cpp.cpp`, and move LLVM IR, object code, and LLVM/JIT host-callable cases to a row that points at `slang-emit-llvm.cpp` and `source/slang-llvm/`. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
