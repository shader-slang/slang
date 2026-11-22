---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T20:05:51Z
target_doc: pipeline/06-emit.md
review_report: ../../reviews/pipeline/06-emit.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/06-emit.md

## Summary

All three findings were verified against the current source and fixed with minimal edits to the CUDA qualifier sentence, the VM backend checklist entry, and the C-like virtual-interface bullet. Breakdown: 3 fixed, 0 rejected, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-emit-cuda.cpp:479 emits `__device__` for Callable entry points and lines 491-497 handle kernel/host decorations before the default `__device__`. | CUDA section: added Callable, kernel and host qualifier branches |
| F-002 | fixed | source/slang/slang-emit.cpp:3819 defines `emitHostVMCode`, which links IR then calls `emitVMByteCodeForEntryPoints` (slang-emit-vm.cpp:1411); slang-code-gen.cpp:1202 calls the wrapper. | Adding-a-backend step: named `emitHostVMCode` as the wrapper of the bytecode emitter |
| F-003 | fixed | source/slang/slang-emit-c-like.h:291-292, 661, 667 show the virtual hooks are the `*Impl` methods behind non-virtual wrappers. | Shared C-like base: named `*Impl` hooks and the wrappers |
