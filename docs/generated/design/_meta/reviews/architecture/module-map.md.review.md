---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:41+00:00
target_doc: architecture/module-map.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 8f8f11ba3fefd6f5527363f5b7ce223022ccac1773c5e77ca1ff8000d572a91d
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for architecture/module-map.md

## Summary
The map is broadly accurate and all checked paths resolve, but it still falls short of the prompt's exhaustive logical-unit inventory. The most important issue is that independently useful watched units remain absent from the `core`, `compiler-core`, and compiler-orchestration tables. Three smaller issues concern one inaccurate AST responsibility, pervasive unlinked source citations, and a conflation of the two core-module embedding stages.

## Items checked
- Read the target, `_common.md`, the per-doc prompt, `architecture/overview.md`, and the resolved watched inputs from `regenerate.py show` at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified 202 inline entries in table `Files` cells in their stated source groups and resolved all 59 Markdown link targets; generated peer-page links point to manifest pages.
- Spot-checked more than 10 factual claims, including the built-in cache format, lexer and diagnostics roles, compile-request/session/module ownership, AST cloning and substitution caching, IR-family counts, emit backends, standard modules, record/replay, WASM bindings, and `slangc`.
- Verified that the body contains no line-number citations, so there were zero source line citations to re-derive.
- Recomputed the watched-path digest, checked all mandatory front-matter fields, and confirmed the 23,719-byte body is below the 32-KB cap.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Group tables, especially lines 35-97 | The document is not the requested exhaustive decomposition. Examples of absent, independently useful watched units include HTTP packet transport, memory file systems, RIFF support, process execution, Metal/Tint downstream compiler discovery, and central orchestration types such as `CodeGenContext`, `TargetProgram`, and `TranslationUnitRequest`. | The manifest describes this page as an inventory of every logical unit at `docs/generated/design/_meta/manifest.yaml:37-43`. Representative omitted declarations are in `source/core/slang-http.h:81-101`, `source/core/slang-memory-file-system.h:11-30`, `source/core/slang-process-util.h:28-41`, `source/compiler-core/slang-metal-compiler.h:10-15`, `source/compiler-core/slang-tint-compiler.h:9-14`, `source/slang/slang-code-gen.h:90-91`, and `source/slang/slang-target-program.h:25-34`. | Add compact rows for these omitted families and perform a watched-file-to-row coverage sweep. Where the size cap discourages one row per pair, use explicit catch-all family rows that name the covered files and responsibilities. |
| F-002 | minor | AST table, line 127 | The responsibility `Layout assigned during AST traversal` misstates `slang-ast-natural-layout`: the implementation recursively computes and caches a type's natural size; it does not assign a layout during a general AST traversal. | `source/slang/slang-ast-natural-layout.h:85-88` exposes `ASTNaturalLayoutContext::calcSize`, and `source/slang/slang-ast-natural-layout.cpp:92-106` computes and caches `NaturalSize`. | Change the responsibility to “Computes and caches natural sizes for AST types.” |
| F-003 | minor | Tables throughout, for example lines 42-50 and 59-76 | Most source filenames are inline code rather than Markdown links. This violates the universal citation rule and weakens a page whose primary purpose is navigation. | `docs/generated/design/_meta/prompts/_common.md:43-45` requires source-file citations to use workspace-relative Markdown links; the report found 202 unlinked inline entries in `Files` cells. | Convert each file or compact file-family citation in `Files` cells to a workspace-relative link, retaining grouped notation where useful. |
| F-004 | minor | Core-module table, line 272 | The single row says both core-module files are “compiled into `libslang`,” conflating the source-embedding target used to bootstrap generation with the generated binary-module embedding target linked as needed into the library. | `source/slang-core-module/CMakeLists.txt:52-89` defines the source-embedding targets; lines 93-95 say that source generates the embeddable core module; lines 198-217 define the separate generated-module embed and no-embed targets. | Split the row into “bootstrap source embedding” (`slang-embedded-core-module-source.cpp`) and “compiled core-module embedding” (`slang-embedded-core-module.cpp`), with their distinct build roles. |

## No-issues notes
- All 59 relative links resolve at the recorded source commit, and every checked `Files` entry names an existing file in its stated group.
- The `slang-ir-*` estimate is accurate at 162 `.cpp` plus 162 headers and is appropriately summarized instead of enumerated.
- Front matter is complete, and the recomputed watched-path digest exactly matches the target document.
