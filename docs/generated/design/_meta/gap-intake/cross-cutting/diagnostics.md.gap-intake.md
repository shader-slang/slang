---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:15:16Z
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 16
actions:
  fixed: 14
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/diagnostics.md

## Summary

Nothing was escalated: every observation that could be checked was
already what the watched source says. Fourteen gaps were fixed with
edits confined to the five sections they were anchored to, one was
rejected as out of scope (the `DIAGNOSTIC_TEST` annotation grammar is
owned by `docs/diagnostics.md` and implemented outside `watched_paths`),
and one was deferred (diagnostic-reachability shadowing lives in the
semantic checker, outside `watched_paths`, and cannot be settled without
running `slangc`). The one `drift-from-source` gap resolved against the
document: the token-paste note loop exists, but only on the legacy
`diagnoseImpl` formatting path, and every catalog entry now takes the
rich path, so the page was describing a rendering the reader will not
see.

Three items need operator attention. (1) The page is now 25995 bytes
against a `size_cap` of 24576; the lint reports this as a warning. The
overflow is entirely gap-driven prose, so either the cap should be
raised to about 28672 or the command-line-facing material should move to
a peer page. (2) `source/slang/slang-preprocessor.cpp` should be added
to `watched_paths`: it owns `#pragma warning` specifier parsing
(`WarningStateTracker`, lines 1188-1210 and 4285-4380), which is the
only user surface for `SourceWarningStateTrackerBase`, and the page can
currently confirm only part of it. (3) Gap `d4aeafd64cde` claims
`-W<name>` is not accepted; the source says a name *is* accepted
(`overrideDiagnostic` branches on a leading digit and otherwise calls
`findDiagnosticByName`), and the reporting test only ever passes
`-W41016`. The page was corrected to the option table's `-W<id>` /
`-Wno-<id>` spelling and now states that either an id or a name
resolves. If a name really is rejected at runtime that is a compiler
bug, not a documentation one; it needs a run to confirm.

## Actions

| Gap ID       | Action                 | Evidence                                                                                                                                                                                                                                                                    | Fix summary                                                                                                                                                          |
| ------------ | ---------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 78544eaa1263 | rejected-out-of-scope  | `docs/diagnostics.md` lines 105-165 own the `DIAGNOSTIC_TEST` directive and annotation grammar; the parser is `tools/slang-test/diagnostic-annotation-util.cpp`, outside this page's `watched_paths`. Step 8 already routes the reader there via `CLAUDE.md`.                  | —                                                                                                                                                                    |
| 11b06b8f801a | fixed                  | `source/slang/slang-rich-diagnostics.cpp:389-433` generates `for (const auto& item : <list>) { DiagnosticNote note; ... result.notes.add(note); }` per variadic note; `source/slang/slang-rich-diagnostics.h:59-81` emits the nested struct + `List<>`; `source/slang/slang-diagnostics.lua:3986-3992`. | added a paragraph under Anatomy stating one note record per supplied item, with the `ambiguous-overload-for-name-with-args` `variadic_note` as the worked example      |
| 3a7efc985e8b | fixed                  | `source/slang/slang-diagnostics.cpp:38` (`addAlias`), `:71-97` (id-or-name resolution); `source/slang/slang-compiler-options.cpp:615-657` routes `DisableWarning(s)`, `EnableWarning`, `WarningsAsErrors` through it.                                                          | named the options that consume a diagnostic name, so the alias keeps `-warnings-disable overlappingBindings` working                                                   |
| a9afaeff7d54 | fixed                  | `source/slang/slang-diagnostics.lua:5896-5946` (`internal` entries, all `99999`) and `:3432-3469` (`fatal` entries); `source/slang/slang-diagnostics.h:43-68` (the ICE macros raise exactly those three); `source/compiler-core/slang-diagnostic-sink.cpp:696-700`.            | added a paragraph mapping each helper to the kind of condition it describes, stating that `internal` entries mean a compiler defect rather than a reproducible input   |
| a08fdce0f5ce | fixed                  | `source/compiler-core/slang-diagnostic-sink.h:118-121` + `slang-diagnostic-sink.cpp:731-737` (tracker consulted for note/warning); `source/slang/slang-diagnostics.lua:780-820` declares `pragma-warning-*` 15611-15616; tests `pragma-warning-disable-suppresses.slang`, `pragma-warning-unknown-specifier-warns.slang`. | named `#pragma warning(push)` / `(pop)` / `(<specifier> : <id-list>)` as the tracker's user surface and noted that `slang-preprocessor.cpp` is unwatched               |
| 53a781a689ef | deferred               | The claimed pre-emption (E30013 before E30020, E30019 before E30024) is decided in `source/slang/slang-check-*.cpp`, outside `watched_paths`; nothing in the watched files states which catalog entries are reachable. Confirming it needs a `slangc` run, impossible here (Linux x86-64 build, arm64 host). | —                                                                                                                                                                    |
| 8ad5729b0ea8 | fixed                  | `source/slang/slang-diagnostics.lua:3972-3977` (`no-applicable-overload-for-name-with-args`, 39999) and `:3986-3992` (`ambiguous-overload-for-name-with-args`, 39999).                                                                                                         | named both 39999 entries in the Error code namespace section (one consolidated edit with 6a2d82fd6cfa)                                                                |
| 6a2d82fd6cfa | fixed                  | `source/compiler-core/slang-diagnostic-sink.cpp:920` (`m_idMap.addIfNotExists`, first entry wins); `source/compiler-core/slang-rich-diagnostics-render.cpp:796-805` renders severity, `[E<code>]`, and message but never the name.                                             | stated that for a multi-bound code the rendered id does not identify the entry and the message text is the only discriminator (consolidated with 8ad5729b0ea8)         |
| 3de1c2fff064 | fixed                  | `source/compiler-core/slang-rich-diagnostics-render.cpp:765-777` special-cases `PathInfo::Type::CommandLine` and omits `line:col`; `source/slang/slang-options.cpp:4962` gives the parse sink the `CommandLineContext` source manager; `slang-diagnostic-sink.cpp:687-690`.    | added a paragraph on the synthetic command-line source and advised pinning CLI-parse diagnostics by error code                                                         |
| 2c0bc580a959 | fixed                  | `source/compiler-core/slang-diagnostic-sink.cpp:647-649` and `:771-772` drop a `Severity::Disable` diagnostic before rendering; `:696-700` aborts on `>= Fatal`; `slang-diagnostic-sink.h:60-79` (`getSeverityName`).                                                          | stated that only five of the six rendered names ever print, that `ignored` names a state not an output, and that fatal/internal abort the compile                      |
| f5bfc8605d11 | fixed                  | `source/slang/slang-diagnostics.cpp:99-109` reports `UnknownDiagnosticName` for a severity mismatch and returns `SLANG_FAIL`; `source/slang/slang-diagnostics.lua:2623-2628` declares it `err ... 31111`; `slang-diagnostic-sink.cpp:655-658` counts it as an error.           | named the rejection (`unknown-diagnostic-name` 31111 with the requested identifier) and noted it is enforced at the option layer, not in `getEffectiveMessageSeverity` |
| 37d42be463f4 | fixed                  | The loop at `source/compiler-core/slang-diagnostic-sink.cpp:455-495` is reached only from `diagnoseImpl` at `:786`; the rich path at `:660-676` calls `renderDiagnostic`; `slang-diagnostic-sink.h:254-258` shows every catalog entry enters via `diagnoseRichImpl`. Source agrees with the observation, so the document was wrong. | rewrote the token-paste sentence to attribute the note loop to the legacy `diagnoseImpl` path and to say the rich path has no such loop                                |
| 78ba541779f9 | fixed                  | `source/slang/slang-options.cpp:1230-1233` registers `-enable-machine-readable-diagnostics`; `:2835-2846` sets `MachineReadableDiagnostics` together with `AlwaysGenerateRichDiagnostics`.                                                                                     | named the command-line spelling next to the flag and the record layout                                                                                                |
| fdd5599154eb | fixed                  | `source/compiler-core/slang-rich-diagnostics-render.cpp:796-805`: severity name, then `"[E"` plus zero-padding to five digits when `code >= 0`, then `": "` and the message.                                                                                                   | added the header form, the always-`E` prefix rule, the negative-code case, and a two-line rendered example                                                             |
| 8f065a228462 | fixed                  | `source/compiler-core/slang-rich-diagnostics-render.cpp:142` selects `s_unicodeGlyphs` vs `s_asciiGlyphs` (`:197-211`) from `enableUnicode`; `slang-diagnostic-sink.h:362-394` derives unicode from the colour predicate unless set explicitly; `source/slang/slang-options.cpp:2847-2868`. | added a paragraph stating that `-diagnostic-color` selects the frame glyph set as well as colour, and that `auto` resolves via the writer's console check              |
| d4aeafd64cde | fixed                  | `source/slang/slang-options.cpp:593-594` registers the options as `-W<id>` / `-Wno-<id>`; `source/slang/slang-compiler-options.cpp:635-644` and `source/slang/slang-diagnostics.cpp:71-97` resolve the operand as an integer id *or* a name.                                    | corrected `-W<name>` to `-W<id>` and stated that the operand may be an id or a diagnostic name                                                                         |
