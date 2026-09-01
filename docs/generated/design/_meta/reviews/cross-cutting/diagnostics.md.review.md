---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:39+00:00
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 72308d5b1cf5b2f873570484f93cd6c423c9d145955c7dd61b2a25d788038770
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 5
severity_breakdown:
  critical: 1
  major: 1
  minor: 3
  nit: 0
---

# Review report for cross-cutting/diagnostics.md

## Summary

The document is broadly accurate and all links resolve, but it has five findings. Most importantly, the add-diagnostic checklist incorrectly recommends the default-enabled `Extra` group for a warning that should not fire by default. The current watched-path set also no longer matches either the prose or the recorded digest.

## Items checked

- Verified source at the recorded commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, which is also current `HEAD`; none of the resolved watched source files differs from that commit.
- Spot-checked more than 20 factual claims, including sink construction and routing, severity overrides, warning groups, Lua validation and generation, prototype-schema inactivity, token-paste notes, TSV rendering, lookup behavior, assertion routing, and the add-diagnostic workflow.
- Verified every line-number citation: the body contains none, so there were zero source line-number citations to re-derive.
- Resolved all 39 Markdown link occurrences at the recorded commit and confirmed `architecture/overview.md` is both present and declared in the manifest.
- Verified all mandatory sections and front-matter keys, ran the document lint successfully, and confirmed the 20,624-byte document is below its 24,576-byte cap.
- Recomputed the current watched-path digest as `1dac728438feb2408c4a0f51ee6ac7303e143cfa77dda7fe254671622fe3bac1`, which does not match the recorded digest.

## Findings

| ID    | Severity | Location                                                                                                  | Description                                                                                                                                                                                                                                                                                                                                 | Evidence                                                                                                                                                                                                                                            | Recommendation                                                                                                                                                                                              |
| ----- | -------- | --------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | critical | `## Adding a new diagnostic`, lines 420-423                                                               | The checklist says a warning that “should not fire by default” may use `extra`, but the sink enables `WarningLevel::Extra` by default. Following this instruction produces a warning that does fire without an opt-in flag.                                                                                                                 | `source/compiler-core/slang-diagnostic-sink.h:503-512` initializes the enabled-group mask with `Extra`; `source/slang/slang-options.cpp:589-593` likewise states that `-Wextra` is on by default.                                                   | Remove `extra` from this step. Tell authors to use `all` or `pedantic` for a warning that must be off by default, and describe `extra` separately as default-enabled.                                       |
| F-002 | major    | Front matter line 6; `## Warning groups`, lines 286-293; `## What is not in this document`, lines 445-451 | The document twice says `include/slang.h`, `slang-options.cpp`, and `slang-compiler-options.cpp` are outside its watched paths, but the current manifest includes all three. Consequently the recorded digest `72308d...` is stale; `regenerate.py digest cross-cutting/diagnostics.md` returns `1dac7284...` for the resolved watched set. | `docs/generated/design/_meta/manifest.yaml:290-295` lists the three files; `regenerate.py show cross-cutting/diagnostics.md` resolves them.                                                                                                         | Delete both stale “not watched” statements, retain the source links, and refresh the generated document's watched-path digest through the normal regeneration/freshness workflow.                           |
| F-003 | minor    | `## Internal-compiler errors`, lines 372-384                                                              | “In debug builds they emit a companion note” incorrectly applies to all three preceding macros. Only `SLANG_INTERNAL_ERROR` and `SLANG_UNIMPLEMENTED` are defined inside the debug conditional; `SLANG_DIAGNOSE_UNEXPECTED` is defined afterward and emits no compiler-location note.                                                       | `source/slang/slang-diagnostics.h:40-68` shows the two debug-only note-emitting definitions and the unconditional `SLANG_DIAGNOSE_UNEXPECTED` definition.                                                                                           | Change the sentence to name only `SLANG_INTERNAL_ERROR` and `SLANG_UNIMPLEMENTED` as emitting debug companion notes.                                                                                        |
| F-004 | minor    | `## Internal-compiler errors`, lines 386-395                                                              | The assertion description has two inaccuracies: release-build `SLANG_ASSERT` expands to `SLANG_ASSUME` rather than consulting `SLANG_ASSERT`, and `SLANG_ASSERT` calls `handleAssert` directly rather than expanding to the separate `SLANG_ASSERT_FAILURE` macro.                                                                          | `source/core/slang-common.h:364-379` defines the debug and release expansions; `source/core/slang-signal.h:31-33` defines `SLANG_ASSERT_FAILURE` independently.                                                                                     | Distinguish debug `SLANG_ASSERT` from release `SLANG_ASSUME`, state that `SLANG_RELEASE_ASSERT` always calls `handleAssert`, and describe `SLANG_ASSERT_FAILURE` as a separate direct `handleAssert` macro. |
| F-005 | minor    | `## Source locations and message rendering`, lines 308-316                                                | The exact TSV record schema is attributed only to `slang-diagnostic-sink.cpp`, which dispatches to a renderer but does not define the schema. The defining `slang-rich-diagnostics-render.cpp` is outside the resolved watched paths, contrary to the prompt's requirement that rendering claims be supported by watched source.            | `source/compiler-core/slang-diagnostic-sink.cpp:661-675` only calls the renderer; `source/compiler-core/slang-rich-diagnostics-render.cpp:886-958` defines the TSV fields; `docs/generated/design/_meta/manifest.yaml:279-295` omits that renderer. | Add `source/compiler-core/slang-rich-diagnostics-render.{h,cpp}` to the watched paths, then link the implementation when stating the TSV schema.                                                            |

## No-issues notes

- The severity and warning-group enum values, public-API synchronization assertions, and default `Extra` group match the source.
- The Lua example, optional locationless diagnostics, warning-level sentinels, generated rich-diagnostic registration, and prototype-schema status match the source.
- Parent-sink routing, effective-severity ordering, token-paste notes, diagnostic lookup behavior, and the TSV field sequence are otherwise accurate.
