---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:33+00:00
target_doc: ir-reference/structure.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
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
  major: 0
  minor: 5
  nit: 0
---

# Review report for ir-reference/structure.md

## Summary
The page is structurally complete, well linked, and generally well aligned with the recorded source revision. Five localized issues remain: four source/contract inaccuracies in the introduction and opcode-origin descriptions, plus an overstatement about how interface members map to requirement entries. The most consequential issue is that the global-state table gives the wrong lowering route for function-static variables and omits function-static constants from `globalConstant`.

## Items checked
- Reviewed the target, `_common.md`, the per-document prompt, all five resolved watched files, and the three `depends_on` documents at source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified all 63 line-number and line-range citations against the cited source revision; the cited locations themselves are accurate.
- Verified more than 20 factual claims across the Lua opcode definitions, C++ wrappers, builder helpers, AST lowering paths, witness lookup behavior, linking behavior, and module-version constants.
- Resolved all relative links and confirmed every referenced generated peer exists; `regenerate.py lint ir-reference/structure.md` passed.
- Checked the required section order, table columns, opcode coverage, hierarchy, front-matter keys, digest shape, identifier/file names, and universal style constraints.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### Global state`, lines 176-178 | The `global_var` row says a function-static local lowers “via `lowerGlobalVarDecl`,” but `visitVarDecl` routes function-static variables through `lowerFunctionStaticVarDecl`. The adjacent `globalConstant` row names only module-scope `static const`, although function-static constants also produce `IRGlobalConstant`. | `source/slang/slang-lower-to-ir.cpp:11927-11938` separates global and function-static routes; `source/slang/slang-lower-to-ir.cpp:11825-11834` routes a function-static `const` through `lowerConstantDeclCommon`, which emits `IRGlobalConstant` at lines 11690-11714. | Change the `global_var` origin to cite `lowerFunctionStaticVarDecl` for function-static mutable locals, and add function-static `const` via `lowerFunctionStaticConstVarDecl` to the `globalConstant` origin. |
| F-002 | minor | `global_generic_param` row, line 179 | The explanation says both `GlobalGenericParamDecl` and `GlobalGenericValueParamDecl` derive from `AggTypeDecl`. Only the type form does; the value form derives from `VarDeclBase`. | `source/slang/slang-ast-decl.h:575` declares `GlobalGenericParamDecl : public AggTypeDecl`; line 583 declares `GlobalGenericValueParamDecl : public VarDeclBase`. | Replace the shared derivation claim with the two correct base classes, or remove the derivation aside. |
| F-003 | minor | `func` row, line 167 | The required AST-origin description omits lambda lowering. A checked lambda synthesizes a `FuncDecl` stored on its `LambdaDecl`, which then follows normal function lowering. | `docs/generated/design/_meta/prompts/ir-reference-structure.md:37-43` explicitly requires lambda lowering in this cell; `source/slang/slang-check-expr.cpp:7933-7939` creates and stores the lambda’s `FuncDecl`. | Add “a synthesized `FuncDecl` for a lambda” to the `func` AST-origin cell. |
| F-004 | minor | `witness_table_entry` vs `interface_req_entry`, lines 389-395 | “One entry per direct interface member” is too broad. Properties and subscripts do not receive entries themselves; each accessor contributes an entry, and default-implementation declarations are skipped. | `source/slang/slang-lower-to-ir.cpp:12086-12108` counts accessor entries separately and skips `InterfaceDefaultImplDecl`; lines 12250-12278 implement the same mapping when entries are created. | Say the list has one entry per requirement-bearing member, with property/subscript accessors contributing their own entries and default-implementation declarations omitted. |
| F-005 | minor | Introduction, lines 12-19 | The first body paragraph states what the page covers, but the intended reader appears only in a separate second paragraph. The common contract requires both in the first paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state coverage and intended reader. | Merge the audience sentence into the opening paragraph. |

## No-issues notes
- Every recorded line-number citation is current at the target document’s `source_commit`.
- The `thisTypeWitness` callout correctly identifies the zero-operand builder/source-schema mismatch.
- The witness-table discussion correctly treats entries as a key-to-value map rather than a positional list.
