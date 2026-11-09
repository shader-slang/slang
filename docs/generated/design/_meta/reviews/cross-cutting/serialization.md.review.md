---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:20:07Z
target_doc: cross-cutting/serialization.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: e47a6f43c0eb2a5586546cd7f7a5b275cfa12cbb91caecfcd280aa2045cbc433
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 2
  major: 0
  minor: 2
  nit: 0
---

# Review report for cross-cutting/serialization.md

## Summary

The core serialization descriptions are well supported, but the document needs corrections before approval. Most importantly, its field-addition guidance incorrectly implies that adding IR information always requires changing the serialization schema. It also incorrectly describes repro handling as removed, overstates freshness coverage, and exceeds the prompt’s length limit.

## Items checked

- Reviewed against the **current working tree**; `git diff` confirmed that `source/` matches the document’s source commit. Confirmed HEAD matches the supplied review commit.
- Checked more than 20 claims covering AST enum generation, generated field visits, generic serializer dispatch, Fossil headers and relative pointers, validation configuration, RIFF chunk codes, embedded downstream blobs, flat-table encoding, null references, allocation bounds, recursion depth, unknown opcodes, source-location reconstruction, and both version checks.
- Verified the sole explicit line-number citation: `-line-directive-mode` is at `source/slang/slang-options.cpp:879`.
- Resolved all 65 Markdown link occurrences, representing 31 distinct targets; checked destination existence at the document’s source commit, peer manifest entries, and the source-location anchor. Consulted the dependency document only for cross-reference context.
- Swept 94 simple backticked identifiers and 31 source-file names; unmatched entries were historical names, metadata vocabulary, or the existing unit-test file outside `source/`.
- Checked all required sections, coverage of serialize-related watched files, front-matter keys and formats, Liquid safety, and document size: 27,202 UTF-8 bytes. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Adding a new serialized field`, lines 473–480 | `Adding information therefore means changing that schema` incorrectly makes a schema change necessary for ordinary IR additions. New operands or instructions can use the existing flat representation; this guidance could cause unnecessary format changes and invalidate existing modules. | `source/slang/slang-serialize-ir.cpp:471` records each instruction’s operand count, and line 488 serializes its type and all operands generically. `source/slang/slang-ir.cpp:4347` represents embedded downstream data using ordinary integer/blob operands. | Explain that ordinary operands and decorations fit the existing schema. Reserve schema changes and serialization-version bumps for changes to the encoding or special payload representation; retain the separate guidance about semantic module versions. |
| F-002 | critical | `## What is not in this document`, line 500 | `The historical repro format. Treat it as removed.` contradicts the retained repro implementation and the page’s earlier explanation that loading and extraction remain available. No specific removed format version is identified. | `source/slang/slang-repro.cpp:1356` implements `ReproUtil::loadState`; line 1404 implements `extractFilesToDirectory`. `CLAUDE.md:362` explicitly retains `-load-repro` and `-extract-repro`. | Replace this bullet with “Repro format details are outside this page’s watched paths.” Keep the distinction between discouraged dumping and retained loading/extraction. |
| F-003 | minor | `## Manifest coverage`, lines 504–506 | `Every source file this page cites for serialization behavior is inside ... watched_paths` is false. The page describes behavior in unwatched files, including Fossil validation and session loading, and even labels some of them unwatched. Changes confined to these files will not trigger this page’s freshness tracking. | `docs/generated/design/_meta/manifest.yaml:1251` lists the watched paths without `source/slang/slang-fossil.h` or `source/slang/slang-session.cpp`; their behavior is discussed at document lines 149–159 and 429–433. | Qualify the coverage statement to describe the watched serialization drivers and explicitly identify these unwatched behavioral dependencies. |
| F-004 | minor | Entire document | The document is 27,202 UTF-8 bytes, exceeding the per-document requirement `Document length under 24 KB`, although it remains below the manifest’s 32,768-byte ceiling. | `docs/generated/design/_meta/prompts/cross-cutting-serialization.md:59` sets the stricter limit. | Reduce the document below 24,000 bytes while retaining required sections; shorten historical backend discussion and repeated validation/version explanations first. |