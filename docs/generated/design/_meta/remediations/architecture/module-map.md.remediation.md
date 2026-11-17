---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:00:00Z
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 2
  escalated: 0
---

# Remediation report for architecture/module-map.md

## Summary

Each finding was checked against the current source. Four were real and fixed in the doc (F-001, F-002, F-003, F-006). Two were deferred (F-004, F-005): one needs a table-wide rewrite that would exceed the size cap, the other a manifest change. The doc grew from 30478 to 31585 bytes against the 32768-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-code-gen.cpp:743-762` adds `slang-rt` only under `_isCPUHostTarget`; CUDA is not host-style | `slang-rt` row now says host-style CPU outputs incl. PyTorch bindings, not CUDA |
| F-002 | fixed | `slang-lookup.h`, `slang-mangle.h`, `slang-parameter-binding.h`, `slang-type-layout.h`, `slang-vm.h` (`ByteCodeInterpreter`, line 72) exist and were absent from the doc | added four compact rows to the compile-request table; wider family audit not attempted to stay within the cap |
| F-003 | fixed | `source/slang-dispatcher/main.cpp:66-75` registers `compile` to `slangc`, `interpret` to `slangi`, plus help and version | heading renamed to command-line dispatcher; responsibility cell rewritten |
| F-004 | deferred | Linking every plain-filename citation would add several KB and exceed the 32768-byte cap (doc is 31585 bytes); needs a table-wide restructure into summary-with-link rows | — |
| F-005 | deferred | `_meta/manifest.yaml:57` watched paths must be expanded to cover the `.h.in`, standard-module sources and glslang version script; manifest edits are outside this remediation | — |
| F-006 | fixed | `source/slang/slang-api.cpp:391-399` allocates a fresh `Slang::Session` per call, so it is not a process-wide singleton | "Process-wide" replaced by "Global-session-scoped" |
