---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: ir-reference/decorations.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 1f43055876c44da0b02f3e913b22e16142720f3f452022bd3fdfcf1687346392
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for ir-reference/decorations.md

## Summary

The page has the required IR-reference sections, valid front matter, and a broad table-oriented catalog of the `Decoration` family. The main remediation issue is that the `C++ wrapper` column does not consistently name the actual C++ `IR*` wrapper structs from `slang-ir-insts.h`; there are also two smaller prompt/style issues around source citation and backend-specific prose.

## Items checked

- Checked the manifest scope from `regenerate.py show ir-reference/decorations.md`, including the per-doc prompt, common contract, dependency pages, and six resolved watched source files.
- Verified the target front matter has all required generated-document keys and copied its `source_commit` / `watched_paths_digest` into this report.
- Resolved the page's markdown links to watched source files, peer generated docs, and `source/slang/slang-ir-glsl-legalize.cpp`.
- Checked the Lua `Decoration` range, including `highLevelDecl`, `layout`, `branch`, `flatten`, `loopControl`, `TargetSpecificDecoration`, `requirePrelude`, `nameHint`, `ResultWitness`, `entryPoint`, `keepAlive`, `BuiltinRequirementDecoration`, autodiff decorations, SPIR-V decorations, and final `BitFieldAccessorDecoration` / `experimentalModule` entries.
- Spot-checked wrapper declarations in `slang-ir-insts.h` for naming/provenance, target-specific, capability, entry-point, linkage, known-builtin, builtin-requirement, and autodiff decorations.
- Spot-checked lowering or builder claims for `nameHint`, `highLevelDecl`, `targetIntrinsic`, `requirePrelude`, `intrinsicOp`, `branch`, `flatten`, `loopControl`, `loopMaxIters`, `entryPoint`, `keepAlive`, and `BuiltinRequirementDecoration`.
- Checked that the table keeps one short summary sentence per row and remains under the manifest size cap by inspection.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes`, `C++ wrapper` column | The wrapper column does not list the exact C++ wrapper struct names required by the IR-reference contract. It uses unprefixed names such as `NameHintDecoration`, `TargetIntrinsicDecoration`, and `BuiltinRequirementDecoration` instead of `IRNameHintDecoration`, `IRTargetIntrinsicDecoration`, and `IRBuiltinRequirementDecoration`; it also uses `—` for entries that do have implicit wrapper structs, such as `KnownBuiltinDecoration`, `SequentialIDDecoration`, and `AutoDiffOriginalValueDecoration`. | `source/slang/slang-ir-insts.h:46` declares `struct IRHighLevelDeclDecoration`; `source/slang/slang-ir-insts.h:190` declares `struct IRNameHintDecoration`; `source/slang/slang-ir-insts.h:122` declares `struct IRTargetIntrinsicDecoration`; `source/slang/slang-ir-insts.h:462` declares `struct IRKnownBuiltinDecoration`; `source/slang/slang-ir-insts.h:470` declares `struct IRBuiltinRequirementDecoration`; `source/slang/slang-ir-insts.h:537` declares `struct IRSequentialIDDecoration`; `source/slang/slang-ir-insts.h:553` declares `struct IRAutoDiffOriginalValueDecoration`. | Replace wrapper cells with the exact `IR*` struct names from `slang-ir-insts.h`; use `—` only for decoration opcodes with no wrapper struct. |
| F-002 | minor | `## Source`, first paragraph after the source links | The source section cites `slang-check-modifier.cpp` as the validator for most AST-side modifiers, but it is plain text rather than a workspace-relative markdown link and it is not part of this document's resolved watched paths. That leaves the claim outside the page's source contract. | `docs/generated/design/_meta/prompts/_common.md:43` says source file citations should use markdown links with workspace-relative paths; `regenerate.py show ir-reference/decorations.md` lists only `slang-ir-insts-info.cpp`, `slang-ir-insts.h`, `slang-ir-insts.lua`, `slang-ir.cpp`, `slang-ir.h`, and `slang-lower-to-ir.cpp` as watched files. | Either remove the `slang-check-modifier.cpp` claim, or add the file to this doc's watched paths and link it as `[slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)`. |
| F-003 | minor | `### glslFragDepthGreater / glslFragDepthLess` | The callout explains GLSL legalization, GLSL emission, and glslang/SPIR-V behavior in detail. The decorations prompt explicitly forbids backend-specific consumption in this page, so this section goes beyond the per-opcode reference scope. | `docs/generated/design/_meta/prompts/ir-reference-decorations.md:75` lists backend-specific consumption of decorations as forbidden content and points to `../pipeline/06-emit.md`; the target doc's callout says the GLSL emitter redeclares `layout(depth_greater)` / `layout(depth_less)` and that glslang maps them to SPIR-V execution modes. | Keep the row-level source/origin fact for these decorations, but shorten the callout or replace the backend details with a link to the relevant pipeline/emission page. |

## No-issues notes

- The required generated-document front matter is present and internally consistent.
- The main generated-doc links in `## See also` resolve to existing peer pages.
- The page remains table-oriented, which matches the per-doc prompt for this large family.
