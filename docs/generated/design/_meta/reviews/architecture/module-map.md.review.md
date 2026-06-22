---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:33+00:00
target_doc: architecture/module-map.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: d8643144402b3b5b8e46aca73d7eff5df8942a1b6227b1a92d42854ab8ea8279
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for architecture/module-map.md

## Summary
The page is broadly accurate and all checked relative links resolve, but it is only partially complete against its prompt contract. The important issue is that several table rows cite files outside this document's watched-path inputs, despite the module-map prompt requiring every table path to be in the watched set.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show architecture/module-map.md` and reviewed the prompt, manifest entry, resolved watched files, and dependency document `architecture/overview.md`.
- Checked front matter for the required generated-doc keys, source commit, warning string, and 64-character hex watched-path digest.
- Checked relative links in the document body, including source links, generated-doc peer links, and handwritten-doc links; no dangling links were found in the final lint run.
- Verified the required module-map structure: level-2 groups, `Logical unit` / `Files` / `Responsibility` tables, source/slang subgrouping, and one-sentence responsibility cells.
- Spot-checked more than 10 source-alignment claims, including `Linkage`, `Session`, `Module`, `DiagnosticSink`, lexer/token files, `FrontEndCompileRequest`, AST file families, semantic-checking files, IR core files, emit backend files, core/GLSL module embedding, standard-module entries, and all prelude headers.
- Checked that no body line-number citations needed verification; the document uses file links without source line anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## source/slang-llvm/` through `## source/slangc/`, lines 278-318 | Several table rows cite concrete source files that are not in this page's watched-path set, such as `source/slang-llvm/slang-llvm.cpp`, `source/slang-glslang/slang-glslang.cpp`, `source/slang-dispatcher/main.cpp`, `source/slang-rt/slang-rt.cpp`, `source/slang-record-replay/replay-context.cpp`, `source/slang-wasm/slang-wasm.cpp`, and `source/slangc/main.cpp`. The links resolve, but the prompt's quality checklist requires every table file path to exist in the watched paths, and the manifest only watches `source/*/CMakeLists.txt` for these sibling subprojects. | `docs/generated/design/_meta/prompts/architecture-module-map.md:58-61` requires every table file path to exist in the watched paths. `docs/generated/design/_meta/manifest.yaml:35-46` watches `source/*/CMakeLists.txt`, `source/slang/slang-*`, `source/compiler-core/slang-*`, `source/core/slang-*`, and `prelude/*.h`, but not the cited sibling-subproject `.cpp` / `.h` files. | Either expand the manifest watched paths to include the cited sibling-subproject source files and regenerate, or revise those rows to cite only files covered by the current watched paths. |

## No-issues notes
- The front matter matches the generated-document contract and the target document's recorded source commit is the current review HEAD.
- The `source/core/`, `source/compiler-core/`, and main `source/slang/` responsibility rows matched the representative headers and source files I spot-checked.
- The prelude table mentions every resolved `prelude/*.h` header.
