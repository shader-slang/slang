---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T15:00:00Z
target_doc: pipeline/06-emit.md
review_report: ../../reviews/pipeline/06-emit.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/06-emit.md

## Summary

Four findings were reviewed. Three were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed: the obsolete watched-set claims around the dispatcher, the blanket statement that every artefact carries post-emit metadata, and the claim that every textual target ships a prelude header. The major finding concerns the front-matter `watched_paths_digest` and was rejected as out of scope. The document was edited.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run: "Do not edit those three fields yourself." The digest is refreshed when the operator marks this page fresh after the edits below. | — |
| F-002 | fixed | The manifest entry for this page now watches `source/slang/slang-code-gen.cpp` and `source/slang/slang-global-session.cpp` alongside `slang-emit.cpp` and the `slang-emit-*` glob, so both stale claims were wrong. Only the `prelude/*.h` contents remain unwatched. | `## Emit dispatcher`: dropped the "not in the watched paths" qualifier and the anchor link. `## Paths outside the watched set`: rewritten to list the current watched set and retain only the `prelude/*.h` gap. |
| F-003 | fixed | Verified at HEAD: `source/slang/slang-emit.cpp:2975` associates metadata on the source artifact and `:3524` on direct SPIR-V, while the HostVM artifact created at `:3581-3582` and `emitLLVMForEntryPoints` at `:3587` do not; `linkedIR.metadata` appears nowhere else in the file. | `## Inputs and outputs`: metadata sentence split out and qualified by emit path. |
| F-004 | fixed | `source/slang/slang-global-session.cpp:125-128` registers language preludes only for CUDA, C++, and HLSL; `emitEntryPointsSourceFromIR` handles Torch and heterogeneous host output separately. The page's own later paragraph already said GLSL, Metal, and WGSL have no `prelude/` header. | `## Preludes`: opening sentence now scopes shipped preludes to the targets in the table and notes the others rely on backend-emitted vocabulary. |
