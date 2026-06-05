---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:19:16+00:00
target_doc: cross-cutting/core-module.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 19ad329c51b4e53e37b131c94d49631623fa525a7de092b35d5852c27a4bca02
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for cross-cutting/core-module.md

## Summary
The document satisfies the requested structure and all relative links resolve at the target source commit. One minor factual issue remains: the core-module inventory names `Result` as a provided type, but the watched meta-Slang files at the target commit do not declare such a type.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/core-module.md` and reviewed the resolved watched paths plus the dependency document `architecture/overview.md`.
- Checked front matter for the required generated-doc keys, source commit shape, warning string, and watched-path digest value.
- Resolved all 41 markdown links in the document against `52339028a2aa703271533454c6b9528a534bac31`; no dangling links were found.
- Verified the required prompt sections: shipped-code families, core module, GLSL module, standard modules, preludes, and building the core module.
- Spot-checked more than 10 source claims, including the `*.meta.slang` file set, the core and GLSL embedding CMake targets, generated module headers, neural standard-module output layout, every listed prelude header, and representative declarations in `core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`, and `glsl.meta.slang`.
- Checked that no body line-number citations needed verification; the document uses file links without source line anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Core module`, lines 100-105 | The page says the core declarations include `Optional`, `Result`, and `Tuple`, but `Result` was not found in the watched meta-Slang sources at the recorded source commit. | `source/slang/core.meta.slang:1790` declares `Optional`, `source/slang/core.meta.slang:1921` declares `Tuple`, and a search of the watched `core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`, and `glsl.meta.slang` files at `52339028a2aa703271533454c6b9528a534bac31` found no `Result` declaration. | Remove `Result` from that sentence, or replace it with a type that is actually declared in the watched meta-Slang files. |

## No-issues notes
- The page mentions every resolved prelude header under `prelude/`.
- The standard-module section correctly identifies `neural` as the only module under `source/standard-modules/` at the target commit.
- The embedding description matches the generator-expression wiring in `source/slang/CMakeLists.txt` and the generated-header flow in `source/slang-core-module/CMakeLists.txt`.
