---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:47:12Z
target_doc: cross-cutting/serialization.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/serialization.md

## Summary

Six gaps were acted on: five fixed, one rejected as out-of-scope;
nothing was deferred and nothing was escalated as a compiler defect.
Both `ambiguous-claim` gaps were resolvable from the watched paths and
neither turned out to be a compiler bug: the source-location question
("which of the two line-start tables does a consumer read?") is
answered by `slang-serialize-source-loc.cpp` splitting the two roles —
physical `LineInfo`s rebuild the line-break array, `AdjustedLineInfo`s
are replayed as `SourceView` entries — and the versioning question is
answered by two genuinely distinct numbers that the page previously
collapsed into one. One `missing-surface` gap was corrected rather than
transcribed: `-embed-downstream-ir` does **not** add a RIFF chunk, so
the sentence the gap proposed would have documented a container
composition that does not exist; the artefact rides inside the IR
payload as an `EmbeddedDownstreamIR` instruction whose blob operand
goes through the flat encoding's string/blob path. The rejection is the
per-target rendering table for emitted `#line` directives, which
`docs/generated/design/pipeline/06-emit.md` already owns. Two operator
items follow. First, the page is now 23824 bytes against its
24576-byte `size_cap`, so the next round has little headroom; the
`## IR flat-module read path` section is the natural split point if the
cap is not raised. Second, three of the five fixes lean on files the
manifest does not watch — `source/slang/slang-ir.h` (the
`k_min/k_maxSupportedModuleVersion` window), `source/slang/slang-ir.cpp`
(the null-type facts behind the flat-table example), and
`source/slang/slang-options.cpp` (the CLI printers) — and each is
either named in the page as being outside the watched set or carried
only by a bundle test's verified `CHECK` lines. Adding `slang-ir.h` to
`watched_paths` would make the versioning paragraph fully self-hosted.

One non-escalated observation for the tests side: the
`[k_minSupportedModuleVersion, k_maxSupportedModuleVersion]` window
that `-get-supported-module-versions` publishes is never compared
against a loaded module's version anywhere in the tree — a grep for
`SupportedModuleVersion` over `source/`, `tools/` and `include/`
returns only the printer in `source/slang/slang-options.cpp:4018-4019`
and the default initializer at `source/slang/slang-ir.h:2292`. That is
not a contradiction with anything the document claimed, so it is
documented as fact rather than escalated, but it is a plausible future
finding if the window is meant to gate loads.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 19d2ab698c00 | fixed | `source/slang/slang-serialize-ir.cpp:458` maps `nullptr` to `-1` once, `:465-481` assigns indices in preorder and accumulates `childCounts`, `:483-490` writes the type-use slot then each operand into the single `operandIndices` list, `:492-514` routes constants to `literals` / `stringLengths`+`stringChars`; `source/slang/slang-serialize-ir.h:67-95` is the module-level reorder that pushes literals and strings to the end. The two null types in the example are `source/slang/slang-ir.cpp:5064` (`_allocateInst<IRModuleInst>(kIROp_ModuleInst, 0)`, no type) and `:2943-2951` (`getType` passes `nullptr` as the type of a basic type), both outside `watched_paths` and therefore not cited in the page. | added a three-instruction worked example with an `instAllocInfo` / `childCounts` / `operandIndices` table, showing `-1` as a null *type* in the type-use slot |
| c6bb1ff84b4e | fixed | `source/slang/slang-ir-insts-stable-names.lua:627` (`["EmbeddedDownstreamIR"] = 648`) shows the payload is an ordinary opcode with a stable name, and `source/slang/slang-serialize-ir.cpp:507-513` is the `kIROp_StringLit` / `kIROp_BlobLit` case its blob operand travels through, so it is serialized inside the IR payload, not as a RIFF chunk. The operand shape is `source/slang/slang-ir-insts.lua:3011` and `source/slang/slang-ir-insts.h:2968`; the CLI surface is the verified `//TEST:COMPILE` (`-embed-downstream-ir`) and <code>CHECK: EmbeddedDownstreamIR(&#123;&#123;[0-9]+&#125;&#125; : Int,</code> lines of `docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang`. | added a paragraph naming `-embed-downstream-ir` and the `-dump-module` rendering, stating that the artefact rides inside the IR payload rather than becoming a chunk |
| 34fc23654a08 | fixed | `source/slang/slang-serialize-source-loc.cpp:84-117` shows a reached line goes into exactly one of the two lists, selected by `sourceView->findEntryIndex(sourceLoc)`; `:281-341` shows the reader rebuilding `lineBreakOffsets` from the *physical* `LineInfo` of both lists; `:343-375` shows each `AdjustedLineInfo` replayed as a `SourceView::Entry` with `m_lineAdjust = adjustedLineIndex - lineIndex` and the overridden path, then `setEntries`. | replaced "both unadjusted and `#line`-adjusted" with the one-record-per-line rule and a paragraph stating which reconstruction each list feeds |
| a5f989fa1d2a | rejected-out-of-scope | `docs/generated/design/pipeline/06-emit.md:68-69` already resolves `LineDirectiveMode` per target (GLSL defaults to `GLSL`, WGSL to `None`) and `:317-320` documents `advanceToSourceLocation` emitting `#line` and the directive styles, so the per-target rendering table belongs there. This page's prompt (`docs/generated/design/_meta/prompts/cross-cutting-serialization.md:37-38`) scopes `## Source-location serialization` to `slang-serialize-source-loc.cpp`, and no emit-side file (`slang-emit-source-writer.cpp`, `slang-emit-c-like.cpp`, `slang-options.cpp`) is in this page's `watched_paths`. | — |
| 0ae384bf3151 | fixed | `source/slang/slang-serialize-ir.cpp:43-44` declares `kSupportedSerializationVersion = 1` / `serializationVersion`, and `:813` is the only version comparison on the read path; `:711` and `:722` serialize `IRModule::m_version` next to the module name in `handleIRModule`, and `:782` hands it back out of `readSerializedModuleInfo` uncompared. The window is `source/slang/slang-ir.h:2260-2261`, printed at `source/slang/slang-options.cpp:4016-4019` (both outside `watched_paths`; the page says so). The two CLI surfaces are the verified <code>CHECK: Module Version: &#123;&#123;[0-9]+&#125;&#125;</code> line of `module-info-name-version.slang` and the `Minimum/Maximum supported version` lines of `supported-module-versions.slang`, both in `docs/generated/tests/design/cross-cutting/serialization/`. | added a paragraph separating the fossil-schema `serializationVersion` from `IRModule::m_version`, naming the two CLI probes and stating that only the former can fail a load |
| ee09f6037ed3 | fixed | `source/slang/slang-serialize-ir.cpp:761-786` (`readSerializedModuleInfo`) returns exactly the compiler version, module version and name that `-get-module-info` prints, without deserializing the instruction graph; the writer/reader command forms are the verified `//TEST:COMPILE: ... -o ....slang-module` plus `//TEST:SIMPLE: -get-module-info` directives of `docs/generated/tests/design/cross-cutting/serialization/module-info-name-version.slang` and the `-dump-module` directive of `embed-downstream-ir-spirv.slang`. | added a "from the command line" paragraph to `## What is serialized` naming the writer and the two readers |
