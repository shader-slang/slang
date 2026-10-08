---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:43+00:00
target_doc: ast-reference/types.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: b05183b09ce7aed8128e45e31d4986249932ca45431a04b94d0ca99f3d163135
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: fail
finding_count: 8
severity_breakdown:
  critical: 0
  major: 5
  minor: 3
  nit: 0
---

# Review report for ast-reference/types.md

## Summary
The page now covers every concrete type class in one table and most checked claims match the recorded source. The most important issue is that its watched-path digest no longer matches the manifest-resolved inputs, while the prose still says the newly watched parser is unwatched. Required details are also missing from several prompt-mandated callouts and cross-references.

## Items checked
- Ran `regenerate.py show ast-reference/types.md` and reviewed the target document, `_common.md`, `ast-reference-types.md`, and dependency docs `ast-reference/base.md` and `ast-reference/values.md`.
- Confirmed all four resolved watched files are unchanged from `53b76e6d3009b8e6434d41573524c7ce5c499d23`, then inspected the type/base/value headers and parser entry points at that commit.
- Compared all 121 table rows with the concrete `FIDDLE()` declarations and all 12 abstract intermediates in `slang-ast-type.h`; coverage and parent classification are complete.
- Spot-checked more than 10 claims: `Val -> Type`, `m_operands`, `getCanonicalType()`, parser entry points, `DeclRefType`, arithmetic accessors, `FuncType` operands, pointer modes, resource handles, existential caches, `ThisType`, `AndType`, and pack operands.
- Resolved all 48 relative-link occurrences and their anchors; every generated peer destination is present in the manifest.
- Found zero line-number citations in the body, so there were no line-number citations to re-derive.
- Confirmed all mandatory front-matter keys, but recomputation produced digest `b1f3c2700c886b77f0e421224e42c78398c812e92f550840904a27c88b810926`, not the recorded value.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | front matter, line 6 | The recorded watched-path digest is stale: it is `b051...3135`, while `regenerate.py digest ast-reference/types.md` resolves to `b1f3...926` with the current manifest and the same source commit. | `docs/generated/design/ast-reference/types.md:6`; `docs/generated/design/_meta/manifest.yaml:401-413`; recomputed digest `b1f3c2700c886b77f0e421224e42c78398c812e92f550840904a27c88b810926`. | Regenerate or refresh this page through the prescribed freshness workflow so front matter records the current resolved watched-path digest. |
| F-002 | minor | `## Source`, lines 35-42 | The page says `slang-parser.cpp` “is not among this page's watched paths,” but it is now explicitly watched. The accompanying recommendation to add it is therefore stale. | `docs/generated/design/_meta/manifest.yaml:401-413` includes `source/slang/slang-parser.cpp`. | Delete the stale parser watch-gap sentence; retain the parser entry-point description. |
| F-003 | minor | `## Nodes`, line 178 | `BorrowInParamType` is described as parsed `in T`, but the type prints as `borrow T`; the parser maps `in` to `InModifier` and the borrow wrapper to `BorrowModifier` via `__constref`. | `source/slang/slang-ast-type.h:890-902`; `source/slang/slang-ast-type.cpp:1407-1410`; `source/slang/slang-parser.cpp:10746-10751`. | Change the summary to an immutable borrow parameter and use `(none)` for grammar unless a direct public grammar production is identified. |
| F-004 | major | `## Nodes`, line 238 | The `ThisType` row links parsed grammar, contrary to the per-page contract requiring checking-only `ThisType` to use `(none)` with a synthesized/checking note. | `docs/generated/design/_meta/prompts/ast-reference-types.md:31-36`; `source/slang/slang-ast-type.h:1332-1343`. | Replace the Grammar cell with `(none)` and state in Summary that checking represents interface/extension self types with this node. |
| F-005 | major | `## See also`, lines 416-432 | The required parsing pipeline page is absent even though this is a parsed AST family. | `docs/generated/design/_meta/prompts/_common.md:119-122`; the current list has no `pipeline/02-parse-ast.md` link. | Add a relative link to `../pipeline/02-parse-ast.md` with a short parsing-oriented description. |
| F-006 | major | `### FuncType`, lines 354-359 | The mandated callout covers parameters, return, and error type, but omits effects/qualifiers and parameter-mode wrappers. | `docs/generated/design/_meta/prompts/ast-reference-types.md:54-55`; `source/slang/slang-ast-type.h:990-1077` documents wrapped parameter types, passing modes, result, and error. | Add a concise explanation that parameter qualifiers are encoded by mode-wrapper types and the error type models explicit failure. |
| F-007 | major | `### Resource and texture type families`, lines 368-378 | The prompt requires this group note to cite the relevant intrinsic definitions in the core module, but the callout contains no module-source link or intrinsic declaration citation. | `docs/generated/design/_meta/prompts/ast-reference-types.md:57-59`; relevant declarations are in `source/slang/core.meta.slang` and `source/slang/hlsl.meta.slang`. | Add direct workspace-relative links to the relevant resource/texture/sampler intrinsic declarations in the module sources. |
| F-008 | minor | `## Notable nodes`, lines 301-311 and 380-401 | The page includes detailed IR identity, lowering-pass, layout, and emit behavior, although AST-family pages forbid IR-level information outside the designated cross-reference. | `docs/generated/design/_meta/prompts/_common.md:131-135`; the target names IR identity and `lowerUntypedResourceHandleToUInt` behavior. | Remove the pass/layout/emit details and leave a short link to `cross-cutting/ir-instructions.md` for IR behavior. |
