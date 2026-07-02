---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:15+00:00
target_doc: ast-reference/modifiers.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: f26e2da5bc0040a0dfba76f743caea5d5bd4cb0277c616e126e276adefc2eb13
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for ast-reference/modifiers.md

## Summary
The page has the right high-level shape, front matter, and source/link scope, but its node table is not reliable for several concrete classes with important fields. The most important issue is that multiple rows say `(no additional state)` even though the watched header declares concrete stored fields that define the node shape.

## Items checked
- Ran `regenerate.py show ast-reference/modifiers.md` and used only the reported prompt, dependencies, and watched files: `source/slang/slang-ast-modifier.h`, `source/slang/slang-ast-base.h`, and `source/slang/slang-parser.cpp`.
- Read the target document front matter/body, `_common.md`, `ast-reference-modifiers.md`, dependency docs `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Checked front matter for all required keys, the recorded target source commit, the warning string, and a 64-character hex watched-path digest.
- Verified the required AST-reference sections are present: Source, Family hierarchy, Nodes, Notable nodes, and See also.
- Spot-checked more than 10 concrete claims against the watched files, including `Modifier`, `ModifiableSyntaxNode::modifiers`, `_parseAttribute`, `UncheckedAttribute`, `GLSLLayoutModifierGroupBegin`, `GLSLLayoutModifierGroupEnd`, `SynthesizedModifier`, `EntryPointAttribute`, `WaveSizeAttribute`, `VulkanHitObjectAttributesAttribute`, `DifferentiableAttribute`, and `MemoryQualifierSetModifier`.
- Resolved the document's workspace-relative source and generated-doc links that were in scope for this page; no broken target paths were found.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, lines 256-301 | Several concrete modifier rows claim `(no additional state)` even though the corresponding classes declare fields that are part of the node shape. Examples include `SynthesizedModifier`, `ExplicitlyDeclaredCapabilityModifier`, `ExtensionExternVarModifier`, and `HasInterfaceDefaultImplModifier`. | `source/slang/slang-ast-modifier.h:180` declares `SynthesizedModifier` with `uint32_t op` and `List<Val*> operands`; `source/slang/slang-ast-modifier.h:196` declares `ExplicitlyDeclaredCapabilityModifier` with `CapabilitySetVal* declaredCapabilityRequirements`; `source/slang/slang-ast-modifier.h:231` declares `ExtensionExternVarModifier` with `DeclRef<Decl> originalDecl`; `source/slang/slang-ast-modifier.h:788` declares `HasInterfaceDefaultImplModifier` with `Decl* defaultImplDecl`. | Update the affected rows' Key fields cells to name the actual stored fields, using the header field names where possible. |
| F-002 | minor | `## Nodes`, lines 408-425 | Several stage and ray-tracing attribute rows summarize fields incorrectly. `WaveSizeAttribute` is listed as `preferred, min, max`, `EntryPointAttribute` as `stage`, and `VulkanHitObjectAttributesAttribute` as `(no additional state)`, but the watched header declares different stored fields. | `source/slang/slang-ast-modifier.h:1270` declares `WaveSizeAttribute` with `IntVal* numLanes`; `source/slang/slang-ast-modifier.h:1306` declares `EntryPointAttribute` with `CapabilitySetVal* capabilitySet`; `source/slang/slang-ast-modifier.h:1362` declares `VulkanHitObjectAttributesAttribute` with `int location`. | Change those Key fields cells to `numLanes: IntVal*`, `capabilitySet: CapabilitySetVal*`, and `location: int` respectively. |
| F-003 | minor | `## Nodes`, lines 450-470 | The differentiability rows omit or misname concrete fields: `DifferentiableAttribute` is described as having `mode`, and `BackwardDifferentiableAttribute` is listed as `(no additional state)`, but neither matches the header. | `source/slang/slang-ast-modifier.h:1647` shows `DifferentiableAttribute` stores associated-value maps and type-registration state rather than a `mode` field; `source/slang/slang-ast-modifier.h:1873` declares `BackwardDifferentiableAttribute` with `int maxOrder`. | Replace `mode` with a compact summary of the actual associated-value/type-registration state, and list `maxOrder: int` for `BackwardDifferentiableAttribute`. |
| F-004 | minor | `## Nodes`, lines 478-489 | A few FFI rows do not match the stored fields. `DllImportAttribute` is summarized as `library`, `AutoPyBindCudaAttribute` as `options`, and `ComInterfaceAttribute` as `UUID, options`; the header has more specific or fewer fields. | `source/slang/slang-ast-modifier.h:1697` declares `DllImportAttribute` with `modulePath` and `functionName`; `source/slang/slang-ast-modifier.h:1736` declares `AutoPyBindCudaAttribute` with `fwdDiffFuncDeclRef` and `bwdDiffFuncDeclRef`; `source/slang/slang-ast-modifier.h:1779` declares `ComInterfaceAttribute` with only `String guid`. | Update those rows to use `modulePath: String, functionName: String`, `fwdDiffFuncDeclRef: DeclRefExpr*, bwdDiffFuncDeclRef: DeclRefExpr*`, and `guid: String`. |

## No-issues notes
- The introduction correctly distinguishes keyword-like modifiers from bracket attributes, and `_parseAttribute` in `slang-parser.cpp` supports the claim that attributes are initially parsed as `UncheckedAttribute`.
- The family hierarchy reflects the important abstract bases in the watched header, including `VisibilityModifier`, `AttributeBase`, `MatrixLayoutModifier`, `InterpolationModeModifier`, and `HLSLSemantic`.
- The GLSL layout-group callout is supported by the parser path that inserts `GLSLLayoutModifierGroupBegin` before parsed layout entries and `GLSLLayoutModifierGroupEnd` afterward.
