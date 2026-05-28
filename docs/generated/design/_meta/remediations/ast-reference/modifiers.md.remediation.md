---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:00:00+00:00
target_doc: ast-reference/modifiers.md
review_report: ../../reviews/ast-reference/modifiers.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/modifiers.md

## Summary

One major finding addressed by adding rows for the five missing
`FIDDLE()` modifier classes. Each was placed alongside its concrete
subclasses so the parent / child relationships remain easy to scan.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-modifier.h:482`, `:515`, `:539`, `:1417`, and `:1455` declare the five classes with `FIDDLE()` (not `FIDDLE(abstract)`), so they qualify as concrete nodes the per-page contract requires to be listed. | Added base rows for `HLSLLayoutSemantic` and `RayPayloadAccessSemantic` to `### HLSL semantics (\`: SV_*\`)`; added a base row for `GLSLPreprocessorDirective` above `GLSLVersionDirective` in `### GLSL preprocessor / layout / format`; added a base row for `HLSLGeometryShaderInputPrimitiveTypeModifier` above the geometry-shader primitives table; added a base row for `HLSLMeshShaderOutputModifier` above the mesh-shader output table. |
