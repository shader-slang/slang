---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:03:00Z
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for architecture/module-map.md

## Summary
Three findings were fixed and one deferred. The coverage gap in F-001 was larger than the review's examples: a basename sweep of `source/core/*.h` and `source/compiler-core/*.h` against the document found roughly 50 and 17 unmentioned units respectively, all now covered by compact family rows plus the three missing orchestration rows. F-002 and F-004 were verified against the cited sources and corrected in place. F-003 is deferred because the fix no longer fits the manifest size cap. The document was edited; it is now 28921 bytes against the 32768-byte cap and `regenerate.py lint` passes.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed and extended: the cited units were absent, as were many peers found by a basename sweep. Roles were read from source, e.g. `source/core/slang-http.h:19` (`HTTPHeader`), `source/core/slang-process-util.h:10,28`, `source/core/slang-persistent-cache.h:12-15`, `source/slang/slang-code-gen.h:90` (`CodeGenContext`), `source/slang/slang-target-program.h:20-34` (`TargetProgram`), `source/slang/slang-translation-unit.h:22`, `source/compiler-core/slang-metal-compiler.h:10`, `source/compiler-core/slang-tint-compiler.h:9`. | Added 14 core rows (I/O, file systems, RIFF, compression, process/platform, HTTP, persistent cache, containers, RTTI, text utilities, portability, profiling, tool support), 8 compiler-core rows plus Metal/Tint/MSVC in the per-vendor row, and 3 orchestration rows (`TranslationUnitRequest`, `CodeGenContext`, `TargetRequest`/`TargetProgram`); extended the hashing, allocation, token, and diagnostic-catalog rows. |
| F-002 | fixed | Confirmed: `source/slang/slang-ast-natural-layout.h:85-88` exposes `ASTNaturalLayoutContext::calcSize` returning a cached `NaturalSize`; there is no general AST traversal that assigns layout. | AST table: responsibility changed to "Computes and caches natural sizes for AST types". |
| F-003 | deferred | The universal rule at `docs/generated/design/_meta/prompts/_common.md:43-45` does require Markdown links, but the document now holds 332 inline file citations; linking them costs roughly 15 KB and the manifest size cap is 32768 bytes with only 3847 bytes free. Fix needs an operator decision: raise `size_cap` for this key to about 49152, or amend the contract to allow one linked representative per family row. | — |
| F-004 | fixed | Confirmed: `source/slang-core-module/CMakeLists.txt:52-95` builds the source-embedding objects consumed by `slang-bootstrap`, while lines 198-217 define the separate `slang-embedded-core-module` / `slang-no-embedded-core-module` targets for the generated binary module. | Core-module table: single row split into "Bootstrap source embedding" and "Compiled core-module embedding" with their distinct build roles. |
