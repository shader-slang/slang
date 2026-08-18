---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:06:00Z
target_doc: architecture/dependency-graph.md
review_report: ../../reviews/architecture/dependency-graph.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/dependency-graph.md

## Summary

All three findings were verified against the build files and fixed. The diagram now keeps to subsystem granularity: the four generated-code targets defined inside `source/slang/` were removed as nodes and are described in prose instead, with the resulting `slang-core-module` to `source/slang/` relationship made explicit as a labelled edge. The source-ownership invariant was qualified for embedded builds, and the stale root-CMake line citation was corrected. The document was edited; it is now 12275 bytes against the 16384-byte cap and `regenerate.py lint` passes.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                                                                                                                 | Fix summary                                                                                                                                                                                                                                                                                                                                                                                                          |
| ---------- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed: the prompt checklist at `docs/generated/design/_meta/prompts/architecture-dependency-graph.md:42-44` requires every node to be a `source/` directory or a `module-map.md` heading, and `slang-fiddle-output`, `slang-capability-defs`, `slang-capability-lookup`, `slang-lookup-tables` are targets declared in `source/slang/CMakeLists.txt` (lines 57, 62-64, 199, 277-278), not subsystems. | Removed the four node declarations and their 14 edges; added a `coreModule` to `slangLib` edge labelled "generated targets"; added a paragraph after the diagram naming the four generated targets, their consumers, and the `source/slang-core-module/` dependency on artefacts owned by `source/slang/`; collapsed the four per-target rows in `## Edge citations` and reworded the `slang` and `slang-wasm` rows. |
| F-002      | fixed  | Confirmed: `source/slang/CMakeLists.txt:322-329` builds `slang-common-objects` from `.`, while the library targets at lines 330-358 are declared `NO_SOURCE` and link it, so "the only target" was wrong for embedded builds.                                                                                                                                                                             | `## Notable invariants`: reworded to a subsystem-level claim and named `slang` (non-embedded) versus `slang-common-objects` (`SLANG_EMBED_CORE_MODULE` on) as the source-owning target.                                                                                                                                                                                                                              |
| F-003      | fixed  | Confirmed: `CMakeLists.txt:366` is `SLANG_ENABLE_RELEASE_DEBUG_INFO`; `SLANG_SLANG_LLVM_FLAVOR` is the `enum_option` identifier at line 386, used through line 401.                                                                                                                                                                                                                                       | Changed "around line 366" to "lines 385-401" in the `slang-llvm` note.                                                                                                                                                                                                                                                                                                                                               |
