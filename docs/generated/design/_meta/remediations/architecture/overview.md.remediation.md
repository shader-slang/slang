---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:57:46Z
target_doc: architecture/overview.md
review_report: ../../reviews/architecture/overview.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for architecture/overview.md

## Summary
The review carried one major finding (F-001) and no others. F-001 is valid: the per-doc checklist clause `docs/generated/design/_meta/prompts/architecture-overview.md:61-62` requires every subsystem in `## Top-level decomposition` to cite a file that exists in the watched paths, yet `regenerate.py show architecture/overview.md` resolves only three `CMakeLists.txt` files, `include/slang.h`, `CLAUDE.md`, and the `slang-compile-request`/`slang-module` header+cpp pairs — none of which cover the many required subsystems (`source/slang-rt/`, `source/slang-wasm/`, `source/slangc/`, `source/slang-llvm/`, `tools/`, `tests/`, `extras/`, `external/`, the prelude/standard-module trees). I deferred F-001 because the only principled fix is a manifest watched-paths expansion, out of scope for a document-only remediation edit. No body edits were made, so `target_doc_source_commit_after` equals `_before`.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | deferred | Valid. The prompt is self-contradictory: section 3 (`docs/generated/design/_meta/prompts/architecture-overview.md:23-40`) mandates describing `source/slang-rt/`, `source/slang-record-replay/`, `source/slang-wasm/`, `source/slangc/`, `source/slang-llvm/`, `source/slang-glslang/`, `source/slang-dispatcher/`, the standard-library dirs, `prelude/`, `tools/`, `tests/`, `extras/`, `external/`, each anchored to a representative file, while the checklist at `:61-62` demands each cite a watched-path file; the manifest (`docs/generated/design/_meta/manifest.yaml:22-31`) resolves only 9 files, none under those dirs. The `_common.md:64-72` "not located in the watched paths" path is inapplicable — the information is present and all 25 cited paths resolve at HEAD (verified), so that boilerplate would falsely claim missing info and gut an accurate, useful doc, also conflicting with the Required-sections clause. The principled fix is a watched-paths expansion, which `_remediate.md:72-78` classes as deferred and which the edit rules forbid here (no manifest/prompt edits). Follow-up: expand this doc's `watched_paths` to add a representative file per subsystem (mirroring `architecture/module-map.md`'s `source/*/CMakeLists.txt` + `source/*/*.{h,cpp}` globs, `manifest.yaml:44-48`), then re-cite the affected entries. | — |
