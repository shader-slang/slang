---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:42+00:00
target_doc: cross-cutting/core-module.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 758f3793c6cde62bb10f3ecad0e65bcabd0d3115b5629165afe8408e4fab2f78
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 1
  minor: 0
  nit: 0
---

# Review report for cross-cutting/core-module.md

## Summary

The document satisfies the required structure, covers all resolved meta-slang and prelude files, and is broadly accurate. Two source-alignment errors remain: the non-embedded build can still compile the core module eagerly and fail during the build, and default preludes are emitted into generated source as embedded text rather than referenced through generated `#include` directives.

## Items checked

- Read `_review.md`, `_common.md`, the per-document prompt, the target document, `architecture/overview.md`, and the complete resolved file list from `regenerate.py show`.
- Verified 31 factual claims at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including meta-module declarations, autodiff types and conformances, standard-module contents and import paths, compiler selection, build products, cache format, and prelude selection.
- Verified every line-number citation in the body; the document contains no explicit source line-number citations.
- Resolved all 59 Markdown-link occurrences (45 distinct destinations) at the recorded commit and confirmed all referenced generated pages are manifest entries.
- Recomputed the watched-path digest, confirmed all mandatory front-matter fields, ran the document lint, and confirmed the 20,211-byte document is below its 24,576-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Core module`, lines 74-80; `## Building the core module`, lines 267-270 and 377-380 | The repeated claim that with `SLANG_EMBED_CORE_MODULE=OFF` meta-source errors surface only at runtime is false for the documented build graph. A non-embedded shared build creates the `ALL` target `generate_core_module_cache`, which depends on `generate_core_module`; both standard modules also depend on `generate_core_module`, and their module targets are `ALL`. Thus the build can run `slang-bootstrap -compile-core-module` and report meta-source errors before any normal session starts, even though C++ source compilation is decoupled from core-module compilation. | `source/slang/CMakeLists.txt:370-404` defines the eager cache target and its dependency on `generate_core_module`; `source/slang-core-module/CMakeLists.txt:141-174` shows that target runs `slang-bootstrap -compile-core-module`; `source/standard-modules/neural/CMakeLists.txt:53-82` and `source/standard-modules/experimental/CMakeLists.txt:46-64` make the standard-module `ALL` targets depend on the same generator target. | Replace all “only at runtime” wording with the narrower invariant: disabling embedding separates core-module compilation from C++ compilation. Explain that the normal full/shared build may still compile the module eagerly for the archive, standard modules, or runtime cache; runtime compilation is the fallback when no valid cache or embedded module is available. |
| F-002 | major | `## Preludes`, lines 235-240 | The document says prelude headers are emitted alongside target output and referenced through `#include "<prelude>"`-style mechanisms. The default compiler path instead embeds each `*-prelude.h` into the compiler as a string, registers the CUDA/C++/HLSL strings on the global session, and writes the selected string directly into generated source. A custom prelude string may itself contain an include, but that is not how the shipped default preludes are brought into scope. | `prelude/CMakeLists.txt:2-20` runs `slang-embed` on each prelude header; `source/slang/slang-global-session.cpp:125-128` registers the embedded default strings; `source/slang/slang-emit.cpp:2937-2951` emits the Torch, host, or language prelude string directly through `sourceWriter.emit(...)`. | Replace the sidecar-header/`#include` description with the embedded-string flow. Note separately that the headers are installed and that callers can override a language prelude with text containing an include if desired. |

## No-issues notes

- All four watched `*.meta.slang` files and all nine resolved `prelude/*.h` files are mentioned.
- The standard-module import paths, build compiler selection, and `-load-core-module` dependency match the two module CMake files.
- The core archive, core embeddable header, and GLSL embeddable header are correctly described as outputs of one `-compile-core-module` command.
