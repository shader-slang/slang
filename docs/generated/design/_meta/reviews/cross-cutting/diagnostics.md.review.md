---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: d89a8a5a54b52ca27bf1790e4d64b99d371b4099edbd608a872c47d632d6eb05
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 3
  minor: 1
  nit: 0
---

# Review report for cross-cutting/diagnostics.md

## Summary

The document has the required high-level structure and all 27 Markdown links resolve, but several tool-facing details do not match the source. The most important issue is the claim that machine-readable diagnostics are JSON-schema based; the sink's machine-readable renderer emits tab-separated records, while the cited JSON files define diagnostics for JSON parsing rather than a diagnostic-output schema.

## Items checked

- Verified the required front matter fields and copied `target_doc_source_commit` / `target_doc_watched_paths_digest` from the target document.
- Used `regenerate.py show cross-cutting/diagnostics.md` to identify the prompt, dependency document, watched paths, and resolved watched files.
- Read `_common.md`, `cross-cutting-diagnostics.md`, the target document, and the dependency document `architecture/overview.md`.
- Spot-checked more than 10 concrete claims against resolved watched files: `slang-diagnostic-sink.h`, `slang-diagnostic-sink.cpp`, `slang-core-diagnostics.h`, `slang-core-diagnostics.cpp`, `slang-diagnostics.h`, `slang-diagnostics.cpp`, `slang-diagnostics.lua`, `diagnostics/type-errors.lua`, `slang-rich-diagnostics.h`, `slang-rich-diagnostics.cpp`, and `source/core/slang-common.h`.
- Resolved all 27 Markdown links in the target document; none were missing.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source locations and message rendering`, lines 185-191 | The paragraph says tools can install a `JSON-emitting writer` and that the JSON schema is governed by `slang-json-diagnostic-defs.h` / `slang-json-diagnostics.cpp`. Those files are a diagnostic catalog for JSON lexing / JSON-RPC errors, not the machine-readable compiler-diagnostic output schema; the sink's machine-readable rich-diagnostic path emits tab-separated records. | `source/compiler-core/slang-diagnostic-sink.h:173` describes `MachineReadableDiagnostics` as `machine-readable TSV format`; `source/compiler-core/slang-rich-diagnostics-render.cpp:893` documents `E<code>\t<severity>\t<filename>...`; `source/compiler-core/slang-json-diagnostic-defs.h:26` begins the `JSON Lexical analysis` diagnostic range. | Replace the JSON-schema claim with a description of `DiagnosticSink::Flag::MachineReadableDiagnostics` and `renderDiagnosticMachineReadable`; do not cite the JSON diagnostic catalog as the output schema. |
| F-002 | major | `## Error codes and the name field`, lines 195-200 | The document states `Every diagnostic has a unique integer id`, but the lookup and helper code explicitly account for multiple diagnostics sharing one numeric id. This matters because suppression by id can map to only the first stored entry. | `source/compiler-core/slang-diagnostic-sink.h:467` says `it is possible for multiple diagnostics to have the same id`; `source/slang/slang-diagnostics-helpers.lua:69` defines an `intentional_shared_code_list`; `source/compiler-core/slang-json-diagnostic-defs.h:41` also contains a duplicate `20011` pair. | Reword the section to say ids live in a shared namespace and are intended to be managed centrally, but some shared ids are intentional; mention that names / flags are safer when a tool needs a precise diagnostic. |
| F-003 | major | `## Internal-compiler errors and assertions`, lines 223-232 | The required prompt asks this section to describe `SLANG_ASSERT`, `SLANG_ASSERT_FAILURE`, and `SLANG_UNREACHABLE`, but the document only covers `SLANG_ASSERT` / `SLANG_RELEASE_ASSERT` and the sink-based `SLANG_INTERNAL_ERROR` family. It omits how `SLANG_ASSERT_FAILURE` and `SLANG_UNREACHABLE` behave. | `docs/generated/design/_meta/prompts/cross-cutting-diagnostics.md:39` requires `SLANG_ASSERT`, `SLANG_ASSERT_FAILURE`, and `SLANG_UNREACHABLE`; `source/core/slang-signal.h:31` defines `SLANG_UNREACHABLE` through `handleSignal`; `source/core/slang-signal.h:33` defines `SLANG_ASSERT_FAILURE` through `handleAssert`. | Add the missing macros to the assertion paragraph, explaining that `SLANG_ASSERT_FAILURE` also routes through `handleAssert`, while `SLANG_UNREACHABLE` routes through `handleSignal`, not the diagnostic sink. |
| F-004 | minor | `## Diagnostic definitions in Lua`, lines 156-175 | The legacy diagnostic schema says entries have `one or more span records`, but source comments and examples allow locationless diagnostics with no span. | `source/slang/slang-diagnostics.lua:85` documents `err(name, code, message, [primary_span], ...)`; `source/slang/slang-diagnostics.lua:87` says `primary_span is optional for locationless diagnostics`; `source/slang/slang-diagnostics.lua:158` defines `cannot-deduce-source-language` with no span. | Change the legacy-schema wording to say legacy diagnostics may have an optional primary span plus additional spans / notes, and call out locationless diagnostics as valid. |

## No-issues notes

- The severity enum and rendered severity-name list match `source/compiler-core/slang-diagnostic-sink.h`.
- The rich diagnostic example matches `source/slang/diagnostics/type-errors.lua`.
- The sink behavior for severity overrides, warning-state tracking, parent sinks, and writer/output-buffer routing is consistent with `slang-diagnostic-sink.h` and `slang-diagnostic-sink.cpp`.
