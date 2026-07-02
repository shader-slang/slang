---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:32:54+00:00
target_doc: ir-reference/values.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 3
  minor: 2
  nit: 0
---

# Review report for ir-reference/values.md

## Summary
The page has the required generated-doc structure, valid front matter, and resolving links, but it is not fully aligned with the value-opcode source. The most important issue is coverage: the opcode tables omit concrete value-family entries that the per-document prompt explicitly calls out or that sit in the documented Lua ranges. I also found a few source-labeling errors in the literal and selection rows.

## Items checked
- Ran `regenerate.py show ir-reference/values.md` and used only the listed prompt, dependency documents, and resolved watched files.
- Read the target document front matter/body, `_common.md`, `ir-reference-values.md`, and the four `depends_on` documents.
- Verified the target front matter contains all required keys, uses source commit `c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8`, and carries a 64-character hex watched-path digest.
- Resolved all 26 relative markdown links in the document body; none were missing in the workspace.
- Checked the required IR-reference sections: `## Source`, `## Family hierarchy`, `## Opcodes`, `## Notable opcodes`, and `## See also`.
- Spot-checked source claims against the watched files, including `Constant`, `Undefined`, `Poison`, `defaultConstruct`, `makeUInt64`, `makeValuePack`, `makeCombinedTextureSampler`, `select`, `logicalAnd`, `CastStorageToLogical`, `CastDescriptorHandleToUInt64`, `NullPtrLiteralExpr`, literal payload storage, `IRBuilder::emitDefaultConstruct`, and `IRBuilder::_findOrEmitConstant`.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes` / `### Aggregate constructors` | The aggregate constructor table omits concrete opcodes in the documented `make*` cluster, including the prompt-required `makeCombinedTextureSampler` and the adjacent `makeValuePack`. | `docs/generated/design/_meta/prompts/ir-reference-values.md:43-47` explicitly includes `makeCombinedTextureSampler`; `source/slang/slang-ir-insts.lua:996-1000` declares `makeStruct`, `makeTuple`, `makeTargetTuple`, `makeValuePack`, and `makeCombinedTextureSampler`; `source/slang/slang-ir.cpp:3246-3249` and `source/slang/slang-ir.cpp:4577-4589` show builder support for the omitted opcodes. | Add rows for `makeValuePack` and `makeCombinedTextureSampler` in the aggregate constructors table, with operand and AST-origin text matching their Lua declarations and builder usage. |
| F-002 | major | `## Opcodes` / `### Conversions` | The conversions table stops before several concrete conversion/pseudo-conversion opcodes in the same source range, so the page is not exhaustive for the conversion cluster. Missing rows include `CastStorageToLogical`, `CastStorageToLogicalDeref`, `CastUInt64ToDescriptorHandle`, `CastDescriptorHandleToUInt64`, `CastDescriptorHandleToResource`, `CastResourceToDescriptorHandle`, and `TreatAsDynamicUniform`. | `source/slang/slang-ir-insts.lua:2596-2612` declares those concrete opcodes; `source/slang/slang-ir.cpp:6576-6595` implements the storage-to-logical builder helpers. | Extend the conversions table with the omitted rows, or move any deliberately out-of-scope resource-specific casts to the correct peer page and cross-link them explicitly so the coverage decision is visible. |
| F-003 | minor | `## Opcodes` / `### Literals (Constant group)` | The literals table marks every literal with `H` and says literals are hoistable, but the Lua entries do not set `hoistable = true`. They are deduplicated through the constant map, which is related but not the `H` opcode flag required by the table contract. | `source/slang/slang-ir-insts.lua:867-880` declares the `Constant` children without `hoistable = true`; `source/slang/slang-ir.h:51-56` defines `H` as `kIROpFlag_Hoistable`; `source/slang/slang-ir.cpp:2301-2393` shows constants are found/emitted through `_findOrEmitConstant` and the constant map. | Clear the `H` flags from literal rows and adjust the prose to say constants are deduplicated by `IRBuilder::_findOrEmitConstant`, not marked with the hoistable opcode flag. |
| F-004 | major | `## Opcodes` / `### Literals (Constant group)` | Two AST-origin cells name AST classes that do not match the dependency/lowering sources: `NullPtrExpr` and `VoidLiteralExpr`. The dependency doc names the null pointer AST node `NullPtrLiteralExpr`, and the watched lowering source shows `void_constant` coming from `getVoidValue` paths such as `NoneLiteralExpr`, not a `VoidLiteralExpr` visitor. | `docs/generated/design/ast-reference/expressions.md:85-86` lists `NullPtrLiteralExpr` and `NoneLiteralExpr`; `source/slang/slang-lower-to-ir.cpp:6843-6850` lowers `NullPtrLiteralExpr` and `NoneLiteralExpr`. | Change `ptr_constant` origin to `NullPtrLiteralExpr` and change `void_constant` origin to `NoneLiteralExpr` / synthesized `IRBuilder::getVoidValue` use, avoiding mismatched class names. |
| F-005 | minor | `## Opcodes` / `### Logical`; `## Notable opcodes` / `### select` | The page describes `select` as the ternary `SelectExpr` lowering generally, but scalar `SelectExpr` inside a function lowers to `ifElse` blocks and a block parameter. Only vector-typed selects or global-scope constants fall back through invoke/intrinsic lowering that can produce the `select` opcode. | `source/slang/slang-lower-to-ir.cpp:6897-6931` branches: non-basic-typed `SelectExpr` and global-scope cases call `visitInvokeExpr`, while scalar in-function `SelectExpr` emits `ifElse`, branches, and a `Param`. | Qualify the `select` AST-origin text and notable callout to say it represents vector/global intrinsic select lowering, while scalar ternary expressions lower through control flow. |

## No-issues notes
- The body includes the required first paragraph and all required section headings from the IR-reference family contract.
- The 26 markdown links are workspace-relative and resolve to source files or generated peer documents.
- The front matter values match the target document's recorded source commit and watched-path digest.
