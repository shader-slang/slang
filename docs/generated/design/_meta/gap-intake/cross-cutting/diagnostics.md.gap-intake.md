---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 4
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/diagnostics.md

## Summary

This pass re-ran intake for the four gaps the ledger marks REOPENED
(recorded as `fixed`, still reported by a regenerated bundle); nothing
was escalated. All four were re-decided as `fixed`, and for each the
page was re-checked against the current tree rather than trusting the
earlier edit. The `a9afaeff7d54` text turned out to be partly wrong
(it said every `internal` entry has code `99999`; two do not), so it
was corrected, and the rendering section gained the new
`-diagnostic-format vs` header form so the "header" claim in
`fdd5599154eb` no longer describes only one of two layouts. The other
two gaps already had accurate text, which was re-verified. Tests could
not be run against this tree (the `slangc` in `build/Release/bin` is a
Linux binary), so behaviour was confirmed by reading source only.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             | Fix summary                                                                                                                                                                                                  |
| ------------ | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| a9afaeff7d54 | fixed  | `source/slang/slang-diagnostics.lua` `internal(` entries: `unimplemented` (6475), `unexpected` (6482), `internal-compiler-error` (6489), `serial-debug-verification-failed` (6508), `spirv-validation-failed` (6515), `no-blocks-or-intrinsic` (6522) all `99999`; `ir-validation-failed` (5229, `40007`) and `missing-existential-bindings-for-parameter` (5769, `50010`); nine `fatal(` entries (3722-6580). `source/slang/slang-diagnostics.h:38-67` shows the three ICE macros raise only the first three; `slang-diagnostic-sink.cpp:697-700` aborts on `>= Fatal`. | corrected the per-helper paragraph in Anatomy: eight `internal` entries listed with their real codes (not all `99999`), stating that reaching one is a compiler defect rather than a supported reproduction |
| 53a781a689ef | fixed  | The Error code namespace paragraph is present and matches source: `30020` is emitted only at `source/slang/slang-check-expr.cpp:6531` inside `visitGetArrayLengthExpr` (6516), whose only producer is `ASTSynthesizer::emitGetArrayLengthExpr` (`source/slang/slang-ast-synthesis.cpp:112`, called from `source/slang/slang-check-decl.cpp:9300`); `Array::getCount` is `__intrinsic_op($(kIROp_GetArrayLength))` (`source/slang/core.meta.slang:2330`); `30024` is emitted at `source/slang/slang-ir-glsl-legalize.cpp:2079`. No edit needed this pass; the earlier compiler-run evidence in the prior report still applies. | — (existing paragraph on catalog entries not implying reachability and the two worked cases re-verified; no text change)                                                                                      |
| 6a2d82fd6cfa | fixed  | `source/compiler-core/slang-diagnostic-sink.cpp:921` (`m_idMap.addIfNotExists`, first entry wins); `source/compiler-core/slang-rich-diagnostics-render.cpp:891` writes `[E<code>]` and the message, never the name; `source/slang/slang-diagnostics.lua` has 27 entries with code `39999`; `slang-diagnostics-helpers.lua:83-106` lists the shared codes. The page already states the message text is the only discriminator for a multi-bound code. | — (existing sentence re-verified in Error code namespace; no text change)                                                                                                                                  |
| fdd5599154eb | fixed  | Default layout: `source/compiler-core/slang-rich-diagnostics-render.cpp:885-892` emits severity name, `[E` + zero-padded 5-digit code when `code >= 0`, `]`, then `: ` and the message, with `E` for every severity. The tree now also has a Visual Studio layout: `renderVisualStudioHeader` (same file, `:805-837`) writes `<file>(<line>,<col>): <severity> E<code>: <message>`, selected by `SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO` (`:143`, option at `source/slang/slang-options.cpp:3003-3020`). | kept the header form and example and added a `-diagnostic-format vs` paragraph so the page no longer implies a single header layout; legacy-path condition updated to mention the format                      |
