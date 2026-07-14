---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:07:22Z
target_doc: ir-reference/decorations.md
review_report: ../../reviews/ir-reference/decorations.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/decorations.md

## Summary

All three findings were fixed in the target document only; no source,
peer, manifest, or front-matter edits. F-001 (major) was the
substantive change: every `C++ wrapper` cell now carries the exact
generated `IR*` struct name, with no `—` cells, because every
decoration opcode produces a wrapper (FIDDLE rule: `IR` + Lua
`struct_name`, or `IR` + capitalized opcode name when no `struct_name`).
F-002 and F-003 (both minor) were resolved by dropping an
out-of-watched-paths source citation and by replacing backend-emission
prose with a pipeline-page link. Action breakdown: 3 fixed, 0
rejected-bogus, 0 rejected-out-of-scope, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Contract `_common.md:234` requires the exact `IRFoo` struct. All 184 wrapper cells validated against `source/slang/slang-ir-insts.h` and the generated `build/source/slang/fiddle/slang-ir-insts.h.fiddle`; the two stale-build names `IRGLSLFragDepthGreater/LessDecoration` confirmed via `kIROp_` tags in `source/slang/slang-ir.cpp:91-92`. Every decoration opcode has a wrapper, so `—` is never correct; em-dash cells (e.g. `KnownBuiltinDecoration`, `SequentialIDDecoration`, `loopCounterDecoration`, FragDepth pair) were filled, and 6 fabricated cross-opcode names (e.g. `BuiltinDecoration` wrongly `IRRequiresNVAPIDecoration`) were corrected to their own `IR*` struct. | `C++ wrapper` column: all 184 rows → exact `IR*` struct; 0 em-dash, 0 fabricated; correct first-letter casing on implicit lowercase wrappers (`IRIgnoreSideEffectsDecoration`, `IRPrimalInstDecoration`, etc.) |
| F-002 | fixed | The clause cited `slang-check-modifier.cpp` as plain text, a file outside this doc's watched paths (`regenerate.py show` lists only the six IR files), conflicting with `_common.md:43`. | `## Source`: removed the `slang-check-modifier.cpp` clause; paragraph now leads with watched `slang-lower-to-ir.cpp` helpers |
| F-003 | fixed | Prompt `ir-reference-decorations.md:75` forbids backend-specific consumption prose (points to `../pipeline/06-emit.md`); the callout described GLSL emission, `layout(depth_greater)`, and glslang/SPIR-V execution-mode mapping. | `### glslFragDepthGreater / glslFragDepthLess`: kept IR-level legalization origin, replaced emitter/glslang/execution-mode detail with a link to `../pipeline/06-emit.md` |
