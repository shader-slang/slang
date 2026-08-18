---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:08:38Z
target_doc: cross-cutting/core-module.md
review_report: ../../reviews/cross-cutting/core-module.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/core-module.md

## Summary

Both findings were verified against the build files and both were
correct, so both were fixed. The critical finding (F-001) was applied
in three places where the page repeated the false "errors surface only
at runtime" claim for `SLANG_EMBED_CORE_MODULE=OFF`; the major finding
(F-002) rewrote the prelude delivery description. No findings were
rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                                                                                                                                                                    | Fix summary                                                                                                                                                                                                                                                                     |
| ---------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Verified: `source/slang/CMakeLists.txt:393-404` makes `generate_core_module_cache` an `ALL` target depending on `generate_core_module`, which runs `slang-bootstrap -compile-core-module` (`source/slang-core-module/CMakeLists.txt:141-166`); `source/standard-modules/neural/CMakeLists.txt:54,82` and `.../experimental/CMakeLists.txt:49` add the same dependency under `ALL`. A non-embedded build therefore compiles the meta-source during the build. | Rewrote the three "surface at runtime" passages (`## Core module`; `## Building the core module` option list and step 4) to say errors surface from the separate `generate_core_module` step that a normal build still runs, with runtime compilation as the no-cache fallback. |
| F-002      | fixed  | Verified: `prelude/CMakeLists.txt:6-21` runs `slang-embed` per `*-prelude.h`; `source/slang/slang-global-session.cpp:126-128` registers the embedded CUDA/C++/HLSL strings; `source/slang/slang-emit.cpp:2940-2951` writes the selected prelude string via `sourceWriter.emit`. The `#include "<prelude>"` description was wrong for the default path.                                                                                                       | Replaced the sidecar-header/`#include` sentence in `## Preludes` with the embed-to-string flow, keeping a note that the headers are installed and a custom prelude string may include one.                                                                                      |
