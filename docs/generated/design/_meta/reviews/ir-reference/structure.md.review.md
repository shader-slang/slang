---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:23:45Z
target_doc: ir-reference/structure.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: e5d3c64488bcb321017f52581b0410875401bb93b51f16f6ab2ccf944dd53b6d
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 1
  minor: 2
  nit: 0
---

# Review report for ir-reference/structure.md

## Summary

The document covers the required structural opcode groups, and its line citations match the current source. The most important correction concerns `StructKey` identity: equal source-level names do not establish equal linkage identities. The opcode tables also omit required producer details and hand-written-wrapper markers.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show ir-reference/structure.md`.
- Verified against the **current working tree**, which the operator confirms equals the document’s source commit throughout `source/`.
- Checked every explicit line-number citation, including parenthesized locations and range endpoints: 88 source positions across eight files.
- Checked the 23 opcode rows against Lua declarations and C++ wrappers, including operand shapes, flags, and concrete-opcode coverage.
- Spot-checked more than 15 behavioral claims: module allocation; entry-block parameters; function-type operand order and trailing attributes; generic returns; global variable pointer types; function-static initialization guards; global generic constraints; interface borrowing; accessor requirements; default-implementation exclusion; associated-type bounds; witness-table construction and deduplication; built-in requirement keys; dump-name escaping; module dump behavior; linking-cache contents; module versions; and alias creation and elimination.
- Resolved all 61 relative-link occurrences to 21 existing destinations; confirmed all 11 generated-document destinations have manifest entries. Consulted dependency documents only for cross-reference checks.
- Checked required sections, table columns, front-matter fields, Mermaid syntax, Liquid safety, and the size cap: 34,226 bytes against 65,536 allowed.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### key / StructKey`, lines 393–397 | The claim that keys for the `same source-level name in two different modules compare equal after linking` conflates spelling with declaration identity. Unrelated fields or requirements with the same name must remain distinct. | `source/slang/slang-lower-to-ir.cpp:1849–1855` caches keys by declaration and constructs linkage with `getMangledName(context->astBuilder, requirementDecl)`, not the source-level name alone. `source/slang/slang-ir-insts.lua:915–923` describes ordinary keys as distinct per declaration and unified by their mangled linkage names. | Say that references to the same declaration unify across modules through the same `key_<mangled>` linkage name; explicitly distinguish unrelated declarations that share a spelling. |
| F-002 | major | `## Opcodes`, especially lines 182, 263, 275–277 | Several AST-origin cells retain the retired `(synthesized)` description or omit the producing function. This leaves required producer information incomplete despite identifiable producers in the watched source. | `_common.md`, IR-reference contract, requires actual producers. `source/slang/slang-lower-to-ir.cpp:15548–15557` emits the hashed-string aggregate in `generateIRForTranslationUnit`; line 10922 creates witness entries in `lowerWitnessTable`; lines 12204 and 12215 create the interface witness and requirement entries in `visitInterfaceDecl`; line 2433 defines `visitTypeEqualityWitness`. | Replace the synthesized placeholders with those named producers. Put `visitTypeEqualityWitness` directly in its AST-origin cell. Audit the remaining cells that name only an AST class, including `param`, `field`, and `SymbolAlias`, and add their lowering functions. |
| F-003 | minor | `## Source`, lines 44–64; `## Opcodes` | The prose identifies hand-written wrappers, but the tables do not mark them and the required count is absent. | `_common.md`, IR-reference contract, requires hand-written-wrapper identification and a count. Examples are `source/slang/slang-ir.h:1850` (`IRFunc`) and `source/slang/slang-ir-insts.h:2353` (`IRWitnessTable`). Of the 23 tabulated wrappers, 19 are hand-written and four are generated. | Mark the 19 hand-written C++ wrapper cells with a consistent footnote marker and add a legend stating the count. Preserve the existing explanation of the four generated wrappers. |
| F-004 | minor | `### module / ModuleInst`, lines 352–354 | `The module's decorations carry import / export information used by the linker` places symbol linkage metadata on the module root. Import/export decorations attach to individual symbols, or their outermost generic wrappers. | `source/slang/slang-lower-to-ir.cpp:1432–1458` attaches linkage to `inInst`, after walking to its outermost generic. `source/slang/slang-ir-link.cpp:2552–2574` reads and updates import/export decorations on the individual cloned instruction. | State that the module contains symbols carrying import/export decorations, and that the linker uses those symbol decorations. Keep serialization described separately as a module-level operation. |