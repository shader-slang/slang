---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:27:59Z
target_doc: architecture/overview.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 0b13514236df58d535ec2ed36e2311b6d01cb5c18433dcafd2320abf4f924a3f
source_commit: cd566e9f0e7783ebfbac93c6e9957b63c4f2e64e
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 0
  minor: 4
  nit: 0
---

# Review report for architecture/overview.md

## Summary

The overview is largely supported by the current source, but needs four localized corrections. The most important is the description of `slang-dispatcher`: it dispatches user-facing CLI commands to Slang executables, rather than providing downstream-compiler support. The standard-module inventory, representative-file coverage, and document length also need attention.

## Items checked

- Read `_review.md`, `_common.md`, and `architecture-overview.md`; ran `regenerate.py show architecture/overview.md`. No dependency documents are declared.
- Verified against the **current working tree**, as requested. HEAD matches the supplied review commit, and comparison with the document’s source commit shows no differences under `source/`.
- Spot-checked more than 15 claims, including compiler output naming, proxy aliases, header staging, mimalloc linkage, core-module cache generation, glslang exports, downstream path-provider behavior, record/replay disabling, WebAssembly methods and descriptor registration, session implementations, translation-unit grouping, module AST/IR storage, compiler-option values, and language-version aliases.
- Verified every body line-number citation: `source/slang/CMakeLists.txt:257–269` and `source/compiler-core/slang-downstream-compiler.h:403`; both match.
- Resolved all 89 relative-link occurrences, covering 70 distinct targets. All eight linked generated Markdown pages have manifest entries.
- Checked required headings, subsystem coverage, front-matter keys and formats, relative-link style, Liquid safety, and UTF-8 size: 22,168 bytes. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### Downstream-compiler shims`, lines 147–149 | `slang-dispatcher` is described as `shared support for dispatching to downstream tools`. Its implementation is a CLI dispatcher for Slang executables, so this description gives readers the wrong responsibility for the directory. | `source/slang-dispatcher/main.cpp:66–76` defines `compile` as “delegates to slangc” and `interpret` as “delegates to slangi”, alongside help/version commands. | Describe it as the `slang` CLI dispatcher that launches `slangc`, `slangi`, and named subcommand executables. |
| F-002 | minor | `### Standard libraries`, lines 123–130 | The description names only `neural` and `experimental` and says `Each subdirectory builds a separate .slang-module artifact`. The current tree also builds `numerics` and `functional`; numerics produces multiple module artifacts. | `source/standard-modules/CMakeLists.txt:54–64` adds all four subdirectories. `source/standard-modules/numerics/CMakeLists.txt:11–20` declares four output module paths; `source/standard-modules/functional/CMakeLists.txt:7` declares `functional.slang-module`. | Include numerics and functional in the brief inventory, and replace the one-artifact-per-subdirectory statement with “These subdirectories build separately distributed serialized modules.” |
| F-003 | minor | `## Top-level decomposition`, lines 99–130 and 203–226 | Several subsystem descriptions link only directories rather than the required representative files: notably prelude, standard modules, tools, tests, and external. The compiler-core examples also do not satisfy the prompt’s watched-file requirement. | `docs/generated/design/_meta/prompts/architecture-overview.md:46–47` requires descriptions anchored to specific representative files; lines 71–72 require a concrete file in the watched paths. The resolved watched set includes `source/compiler-core/CMakeLists.txt`, which this subsystem bullet does not cite. | Add a representative file link for each uncovered subsystem and cite `source/compiler-core/CMakeLists.txt` for compiler-core. Identify the additional representative paths that need adding to the manifest’s watched set. |
| F-004 | minor | Whole document, especially `## Where the public API lives` | The document is 22,168 UTF-8 bytes, exceeding the per-document prompt’s `under 16 KB` requirement. It remains below the manifest’s separate 24,576-byte cap. | `docs/generated/design/_meta/prompts/architecture-overview.md:75` explicitly requires “Document length under 16 KB.” | Shorten the overview below 16,000 bytes, primarily by condensing detailed API migration history and individual feature examples while retaining every required section. |