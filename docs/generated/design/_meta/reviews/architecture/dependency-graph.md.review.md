---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:27:47Z
target_doc: architecture/dependency-graph.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: c607137db619e7fb3fa941a06e151cddf9fdb533d8005452d73cf7e068cf805a
source_commit: cd566e9f0e7783ebfbac93c6e9957b63c4f2e64e
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 0
  minor: 1
  nit: 0
---

# Review report for architecture/dependency-graph.md

## Summary

The document largely matches the current build configuration and satisfies the required structure. Its most important error describes `prelude` as an include-only dependency, although it supplies generated object code. The public-header invariant also cites a policy file that does not state the claimed rule.

## Items checked

- Reviewed against the **current working tree** as requested; confirmed that `source/` has no diff against the document’s source commit and that HEAD matches the supplied review commit.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show architecture/dependency-graph.md`.
- Spot-checked more than 20 claims, including all diagram link edges, generated capability targets, conditional record/replay inclusion, embedded-module selection, shared compiler objects, cache generation, runtime source reuse, allocator gates, glslang exports, and the tools-header dependency.
- Verified all nine line-number citation occurrences: `source/slang/CMakeLists.txt` 184–200 twice and 292; root `CMakeLists.txt` 235–239, 471, 488, and 726–738; `source/slangc/CMakeLists.txt` 1–12 and 14–24. The LLVM option starts at 472–473 rather than 471, within the review’s tolerance.
- Resolved all 40 relative links, covering 19 distinct destinations. Confirmed both referenced generated pages exist in the manifest; used `module-map.md` only for cross-reference and subsystem-coverage checks.
- Checked required sections, all 14 subsystem nodes, Mermaid conventions, front-matter keys and formats, and the 15,363-byte document against its 16,384-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Edge citations`, line 124 | The statement that `prelude` is “a private include dep, not a static link” misrepresents this dependency as header-only. The target compiles generated sources into an object library consumed by the compiler. | `prelude/CMakeLists.txt:9–20` generates `.cpp` files from prelude headers; lines 23–29 declare `OBJECT` with `EXPLICIT_SOURCE ${prelude_source}`. `source/slang/CMakeLists.txt:303–307` lists `prelude` under `LINK_WITH_PRIVATE`. | Replace the include-only explanation with “`prelude` is an object library containing generated embeddings of prelude headers, linked privately into the compiler.” Retain the diagram edge and cite `prelude/CMakeLists.txt`. |
| F-002 | minor | `## Notable invariants`, lines 257–261 | The public-header rule is attributed to `CLAUDE.md`, but that file does not state that public headers must not include private source headers. Consequently, this invariant lacks the demonstrating build-file or header citation required by the prompt. | `CLAUDE.md:456–459` requires ABI and source compatibility, not the asserted include restriction. `source/slang/CMakeLists.txt:315` exposes `${slang_SOURCE_DIR}/include` publicly, which supports the intended public include boundary but does not establish the claimed policy quotation. | Remove the unsupported attribution to `CLAUDE.md`. Describe the public include boundary demonstrated by `source/slang/CMakeLists.txt:315` and cite it; distinguish that observed build interface from any broader project policy. |