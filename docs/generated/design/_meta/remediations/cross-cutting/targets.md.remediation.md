---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:59:34Z
target_doc: cross-cutting/targets.md
review_report: ../../reviews/cross-cutting/targets.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/targets.md

## Summary
One finding (F-001, major), fixed. F-001 correctly identified that the reviewed `C++ host` row grouped the object-code and host-callable `CodeGenTarget` cases under `slang-emit-cpp.cpp` and described them as downstream-compiled, whereas `emitEntryPointForTarget` at `source/slang/slang-code-gen.cpp:1168-1185` dispatches those cases to `emitLLVMForEntryPoints` (`source/slang/slang-emit-llvm.cpp`) when `isCPUTargetViaLLVM(getTargetReq())` holds, and only falls back to the downstream C++ path otherwise. The `## Targets` table now splits these: `C++ host` carries only `SLANG_HOST_CPP_SOURCE` (`slang-emit-cpp.cpp`), and a new `CPU binaries / host-callable` row carries the object-code, host-callable, shared-library, executable, and LLVM-IR formats with the LLVM-vs-downstream dispatch noted. No front-matter, peer doc, source, or prompt was touched.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified against source: `source/slang/slang-code-gen.cpp:1177-1184` routes `Shader/HostObjectCode`, `Shader/HostHostCallable`, shared-library and executable cases to `emitLLVMForEntryPoints` (`source/slang/slang-emit.cpp:3360-3389`; object code -> `emitLLVMObjectFromIR`, host-callable -> `emitLLVMJITFromIR`) when `isCPUTargetViaLLVM`, else to the downstream C++ path. The reviewed single `slang-emit-cpp.cpp` attribution was wrong. In-scope: the `## Targets` table is required structure and the contract requires each row to point at the correct emit file. | Split `C++ host` row into `C++ host` (`SLANG_HOST_CPP_SOURCE` -> `slang-emit-cpp.cpp`) and `CPU binaries / host-callable` pointing at `slang-emit-llvm.cpp` via `emitLLVMForEntryPoints` when `isCPUTargetViaLLVM`, else downstream C++; folded the former standalone LLVM row in. |
