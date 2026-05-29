---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T21:00:00+00:00
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/module-map.md

## Summary

Both findings addressed: the manifest's `watched_paths` for the
module-map page now covers `*.cpp` files in `source/{core,
compiler-core, slang}/` so cited rows are inside the watched set,
and the `Linkage` row points at `slang-session.h` / `.cpp` while a
new `Linkable components` row covers the `slang-linkable.h` cluster.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Runbook "Manifest gaps" pattern: the page legitimately cites `slang-*.cpp` implementation files but they were not watched. | Expanded `architecture/module-map.md` `watched_paths` in `_meta/manifest.yaml` to add `source/slang/slang-*.cpp`, `source/compiler-core/slang-*.cpp`, and `source/core/slang-*.cpp`. |
| F-002 | fixed | `Linkage` is declared in `slang-session.h`, not `slang-linkable.h`; the previous row mixed up the two concerns. | Split the `Linkage` row into three rows: `Linkage` (citing `slang-session.h` / `.cpp` as the per-configuration scope behind `slang::ISession`), `Linkable components` (citing `slang-linkable.h` / `slang-linkable-impl.cpp` for the `IComponentType` cluster), and `Session (global)` (citing `slang-global-session.h` / `.cpp` for the process-wide singleton). |
