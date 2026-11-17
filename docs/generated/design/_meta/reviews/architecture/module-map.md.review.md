---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:27:59Z
target_doc: architecture/module-map.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: ad5cfc68952f75bc650d1948578b7524c289308afe465ed1d94f5fa2ddb91081
source_commit: cd566e9f0e7783ebfbac93c6e9957b63c4f2e64e
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 1
  major: 1
  minor: 4
  nit: 0
---

# Review report for architecture/module-map.md

## Summary

The module map provides useful navigation, but needs corrections and additional coverage before satisfying its generation contract. The most consequential error identifies CUDA output as a consumer of `slang-rt`, although runtime linking is gated on host-style targets. Several substantial compiler units are also absent from the promised exhaustive inventory.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show architecture/module-map.md`.
- Verified against the **current working tree**. The source comparison against the document’s recorded commit produced no differences under `source/`, consistent with the supplied equivalence.
- Resolved all 61 Markdown link occurrences (57 distinct destinations); all exist, and all linked generated pages have manifest entries.
- Searched the entire body for line-number citations: none appear.
- Spot-checked more than 15 claims, including built-in cache timestamp encoding, persistent-cache locking/LRU behavior, inline dictionary storage, AST cloning, substitution caching, IR file counts, switch threading, HLSL barrier helpers, functional/numerics module attributes, standard-module configuration, core-module embedding, mimalloc integration, record/replay inclusion, session interfaces, dispatcher behavior, and runtime linking.
- Confirmed the IR-family count: 167 `.cpp` and 167 `.h` files.
- Compared subsystem coverage and session/runtime descriptions with dependency document `architecture/overview.md`, solely for cross-reference checks.
- Checked required front-matter fields, table structure, source-link conventions, and document size: 30,478 bytes against the 32,768-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## source/slang-rt/`, line 367 | `Runtime library used by emitted CPU/Torch/CUDA targets` incorrectly includes CUDA. This misstates a build/runtime dependency. | `source/slang/slang-code-gen.cpp:354` defines `_isCPUHostTarget` using `ArtifactStyle::Host`; lines 743–762 add `slang-rt` only under that predicate. The dependency overview’s “Runtime and bindings” section also explicitly excludes CUDA source. | Describe `slang-rt` as supporting host-style CPU outputs, including PyTorch bindings; remove CUDA from the consumer list. |
| F-002 | major | `## source/slang/`, lines 101–298 | The promised exhaustive decomposition omits substantial non-pass units: name lookup, mangling, parameter binding/type layout, and the bytecode interpreter. Listing VM emission does not identify the interpreter implementation. | `source/slang/slang-lookup.h:19` declares scope lookup; `source/slang/slang-mangle.h:15` declares mangling; `source/slang/slang-parameter-binding.h:14` describes register assignment; `source/slang/slang-type-layout.h:28` begins layout-rule definitions; `source/slang/slang-vm.h:72` declares `ByteCodeInterpreter`. These are within the watched paths. | Add compact logical-unit rows covering `slang-lookup.*`, `slang-mangle.*`, `slang-parameter-binding.*`, `slang-type-layout.*`, and the `slang-vm*` family. Audit remaining non-pass families for coverage, shortening existing enumeration to retain the size cap. |
| F-003 | minor | Dispatcher heading and row, lines 357–361 | `downstream-tool dispatcher` misidentifies the primary role of this executable: it dispatches Slang CLI commands to sibling executables. | `source/slang-dispatcher/main.cpp:66` registers `compile` → `slangc` and `interpret` → `slangi`, alongside help/version commands. | Rename the heading to “command-line dispatcher” and describe the `slangc`/`slangi` delegation in the responsibility cell. |
| F-004 | minor | Source tables throughout, e.g. lines 43–62 and 137–153 | Many source citations are plain backticked filenames, leaving whole rows without navigable source links. This violates the universal requirement to cite source files using Markdown links. | `docs/generated/design/_meta/prompts/_common.md`, “Universal style rules”: source-file citations must use Markdown links. Examples include the strings, allocation, AST builder, and cloning rows. | Convert cited filenames to relative Markdown links; for summarized families, provide a linked representative file and clearly identify the family. |
| F-005 | minor | Standard-module rows, lines 324–328; glslang row, line 355 | Some table paths fall outside the resolved watched paths, contrary to the document-specific checklist. Examples include `slang-standard-module-config.h.in`, nested standard-module sources, and `slang-glslang.version-script`. Their changes therefore do not trigger this page’s freshness tracking. | `docs/generated/design/_meta/manifest.yaml:57` watches only `source/*/CMakeLists.txt`, `prelude/*.h`, `source/*/*.h`, and `source/*/*.cpp`. `docs/generated/design/_meta/prompts/architecture-module-map.md:60` requires table paths to exist within watched paths. | Expand this manifest entry to include the configuration template, described standard-module source files, and glslang version script; regenerate with the expanded inputs. |
| F-006 | minor | Compile-request orchestration, line 120 | `Process-wide Session class` implies a single process-wide instance, while independent global sessions can coexist. | `source/slang/slang-api.cpp:391` implements `slang_createGlobalSessionWithoutCoreModule`; line 399 allocates a fresh `Slang::Session` for each call. The dependency overview explicitly distinguishes global-session scope from process-wide uniqueness. | Replace “Process-wide” with “Global-session-scoped” in the responsibility cell. |