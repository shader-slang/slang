---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:59Z
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 0, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 1, escalated: 0 }
---

# Remediation report for architecture/module-map.md

## Summary

The review contained one major finding (F-001). It is factually correct
and in-contract, so it is neither rejected-bogus nor rejected-out-of-scope;
however, a clean resolution requires expanding the manifest watched paths,
which is beyond this remediation cycle. The finding is therefore deferred,
and the target document was not edited.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | deferred | Verified correct: the rows in `## source/slang-llvm/` through `## source/slangc/` link concrete `.cpp`/`.h` files (e.g. `source/slang-llvm/slang-llvm.cpp`, `source/slangc/main.cpp`) that exist on disk but are NOT in the resolved watched set — `regenerate.py show` resolves only each sibling subproject's `CMakeLists.txt` (via `source/*/CMakeLists.txt`), and the module-map prompt checklist (`prompts/architecture-module-map.md:60`) requires every table file path to be in the watched paths. The recommendation's primary option is manifest watched-paths expansion, an explicit deferred trigger. Its alternative (revise rows to cite only watched files) cannot fully resolve the finding: `source/slang-llvm/` and `source/slang-record-replay/` have NO `CMakeLists.txt` at HEAD (confirmed via `git ls-files`), so no file under them is in the watched set, yet prompt checklist line 67 still requires a level-2 section per overview logical-unit group; a partial in-doc rewrite would degrade those sections without satisfying the checklist. Blocker: the manifest `watched_paths` for `architecture/module-map.md` must add the sibling-subproject `.cpp`/`.h` sources, then regenerate. Follow-up: expand `manifest.yaml:37-46` and re-run the generation stage. | — |
