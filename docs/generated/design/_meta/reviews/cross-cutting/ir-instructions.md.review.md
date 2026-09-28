---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:20:31+00:00
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 32a6f83c708fb280660629cff147cc6b41bd0816fc7f889340630eb73cb6b9f1
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 0
  major: 2
  minor: 5
  nit: 0
---

# Review report for cross-cutting/ir-instructions.md

## Summary
The page is well linked and broadly source-aligned, but it has two material accuracy problems: several decoration wrappers are presented as Lua opcode names, and module-local hoistable deduplication is described as if one instruction identity spans modules. Five smaller issues affect operand shapes, HLSL spelling, workflow attribution, and one required decorations-contract note.

## Items checked
- Reviewed the target, `_common.md`, its per-document prompt, `regenerate.py show` output, all four resolved watched files, and dependency `architecture/overview.md` at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified all four explicit line-number citations: `slang-ir.cpp:9888`, `slang-ir-insts.lua:1453`, `slang-lower-to-ir.cpp:6592`, and `slang-ir-lower-expand-type.cpp:107`.
- Resolved all 63 markdown links (32 unique targets) at the recorded commit; none dangle, and every generated-doc target exists.
- Spot-checked more than 10 factual claims, including schema defaults and inheritance, generated wrappers, flag values, `Vec`/`Mat`/`Ptr` operands, work-graph record count, generic `return_val` behavior, the two `yield` producers, module versions, stable-name counts, serialization conversion, and the stable-name checker workflow.
- Recomputed the watched-path digest exactly and ran the document lint successfully; the front matter has every required field.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Decorations`, lines 244-248; `## Decorations`, lines 321-325 | The catalog calls `NameHintDecoration`, `KeepAliveDecoration`, `TargetIntrinsicDecoration`, and `EntryPointDecoration` opcodes, but these are `struct_name` values; the Lua opcodes are `nameHint`, `keepAlive`, `targetIntrinsic`, and `entryPoint`. Some listed operand names likewise differ from the schema. | `source/slang/slang-ir-insts.lua:1793-1795`, `1827`, `2087-2093`, and `2157-2159` declare the lowercase opcode keys and wrapper names. | Put the lowercase Lua keys in the Opcode column, retain the decoration names in `struct_name`, use the source operand spellings, and call the `*Decoration` names C++ wrappers in the prose. |
| F-002 | minor | `### Value instructions`, line 192 | The `constexprAdd` through `constexprEnumCast` row says the operands are variadic, but every opcode in that range has a fixed one-, two-, or three-operand schema. | `source/slang/slang-ir-insts.lua:3408-3437` declares explicit fixed operand lists for every `constexpr*` opcode. | Replace `(variadic)` with a compact fixed-arity summary such as `1-3 fixed operands; see Lua entries`. |
| F-003 | minor | `### Resource and shader-IO opcodes`, line 262 | The wave row labels `waveGetActiveMask`, `waveMaskBallot`, and related examples variadic, although the named examples are nullary and binary respectively. | `source/slang/slang-ir-insts.lua:1631-1635` declares `waveGetActiveMask = {}` and `waveMaskBallot` with `mask, condition`. | State that operand shape varies by opcode, or list the two shown shapes explicitly. |
| F-004 | major | `## Hoistable / global / deduplicated values`, lines 308-317 | The statement that one built-in requirement-key instruction is returned “regardless of which ... module asks” overstates identity across modules. Deduplication is owned by an `IRModule`; separately resident modules have separate arenas and deduplication maps. | `source/slang/slang-ir-insts.h:3176-3183` binds each builder to its module's context; `source/slang/slang-ir.h:2012-2021` says the context is owned by `IRModule`; `source/slang/slang-ir.cpp:2865-2869` performs lookup in that context's map. | Scope the guarantee to repeated requests in one destination `IRModule`, and explain that imported instructions are deduplicated after entering that module rather than sharing pointer identity across modules. |
| F-005 | minor | `### Decorations`, line 250 | The node-launch row says HLSL emission re-spells the mode as a named constant, but the emitter writes it as a quoted attribute string. | `source/slang/slang-emit-hlsl.cpp:589-591` emits `[NodeLaunch(\"`, the stored string, then `\")]`. | Change “HLSL named constant” to “quoted HLSL attribute string.” |
| F-006 | minor | `## Adding a new opcode`, lines 390-395 | The page says `check-inst-version-changes.sh` comments on a PR. The script explicitly makes no API call; it writes an artifact that a privileged `workflow_run` job consumes and posts. | `extras/check-inst-version-changes.sh:20-23` describes the split, and lines `156-169` write `pr-number.txt` and `comment-body.txt`. | Attribute artifact creation to the script and PR comment posting to the consuming workflow. |
| F-007 | minor | `## Decorations`, lines 319-333 | The required prompt asks for a cautious note about the planned migration toward decorations-as-instructions, but this section omits it. This needs qualification because the current source already defines decorations as instructions while the older design note still describes that as future work. | `docs/generated/design/_meta/prompts/cross-cutting-ir-instructions.md:58-60` requires the note; `docs/design/ir.md:61` contains the older plan; `source/slang/slang-ir-insts.h:34-39` currently defines `IRDecoration : IRInst`. | Add one sentence distinguishing the historical design-note wording from the current implementation, without presenting the stale plan as an active roadmap. |
