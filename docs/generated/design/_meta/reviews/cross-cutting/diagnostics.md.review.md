---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:19:16+00:00
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 24e450b5284e7561c4747314f08b296074a7d11c69ff4f5fd11f38e74126be29
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

# Review report for cross-cutting/diagnostics.md

## Summary
The page covers the requested diagnostics topics and all relative links resolve at the recorded source commit. One minor source-alignment issue remains: the assertion paragraph implies release assertions usually report through `DiagnosticSink`, but the watched assertion macro definition calls the assert handler directly and does not take a sink.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/diagnostics.md` and reviewed the resolved watched paths plus the dependency document `architecture/overview.md`.
- Checked front matter for the required generated-doc keys, source commit shape, warning string, and watched-path digest value.
- Resolved all 27 markdown links in the document against `52339028a2aa703271533454c6b9528a534bac31`; no dangling links were found.
- Verified the required prompt sections: `DiagnosticSink`, diagnostic definitions, severity levels, source locations and rendering, error codes, internal compiler errors, adding diagnostics, and out-of-scope notes.
- Spot-checked more than 10 source claims, including `Severity`, `Diagnostic`, `DiagnosticInfo`, `DiagnosticSink` source-manager storage, warning-state tracking, diagnostic overrides, legacy Lua helper loading, the rich `argument_type_mismatch` entry, internal-error macros, and assertion macro definitions.
- Checked that no body line-number citations needed verification; the document uses file links without source line anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Internal-compiler errors and assertions`, lines 216-223 | The page says a release assert that fires typically calls into the sink before terminating, but the watched assertion macro expands to `::Slang::handleAssert` and carries no `DiagnosticSink` argument. That makes the described sink interaction unsupported by the watched source. | `source/core/slang-common.h:375` defines `SLANG_RELEASE_ASSERT` and `source/core/slang-common.h:379` calls `::Slang::handleAssert`. The sink-based internal-error path is separate in `source/slang/slang-diagnostics.h:43`, where `SLANG_INTERNAL_ERROR` explicitly calls `(sink)->diagnoseRaw` and `(sink)->diagnose`. | Remove the release-assert sentence about typically calling into the sink, or narrow it to the sink-based macros `SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and `SLANG_DIAGNOSE_UNEXPECTED`. |

## No-issues notes
- The `Severity` enum and rendered severity names match `source/compiler-core/slang-diagnostic-sink.h`.
- The rich diagnostic example matches the `argument_type_mismatch` entry in `source/slang/diagnostics/type-errors.lua`.
- The legacy diagnostic example matches the `function return type mismatch` entry in `source/slang/slang-diagnostics.lua`.
