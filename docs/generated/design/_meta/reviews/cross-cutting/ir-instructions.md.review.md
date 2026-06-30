---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:05+00:00
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 48ed6f3a5c2185f6c92001fe814211a9e9a1c534cd93f7d7a112e99b17b02e52
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for cross-cutting/ir-instructions.md

## Summary
The IR instruction catalog now satisfies the required structure and the representative opcode tables are broadly aligned with `slang-ir-insts.lua`. I found one minor source-citation issue: a helper is attributed to `slang-ir.cpp`, but its inline definition lives in `slang-ir-insts.h`.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/ir-instructions.md` and reviewed the per-doc prompt, `_common.md`, resolved watched files, and dependency `architecture/overview.md`.
- Checked front matter for required keys, source commit, watched-path digest, and warning string.
- Verified the required sections: `## Source`, `## Schema`, `## Instruction families`, `## Hoistable / global / deduplicated values`, `## Decorations`, `## Module versioning and opcode insertion`, and `## Adding a new opcode`.
- Spot-checked more than 10 source claims across `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h`, and `slang-ir.cpp`, including schema fields, representative opcodes, flag bits, decoration attachment, module-version constants, and `builtinRequirementKey`.
- Checked relative links and peer links used by the page; no unresolved target was found.
- Checked that the body has no source line-number citations requiring line-by-line verification.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Hoistable / global / deduplicated values`, lines 247-256 | The page cites `IRBuilder::getBuiltinRequirementKey(kind)` as being in `slang-ir.cpp`, but the helper is defined inline in `slang-ir-insts.h`. A reader following the current citation will not find the named helper in the cited source file. | `source/slang/slang-ir-insts.h:3489` documents the helper and `source/slang/slang-ir-insts.h:3492` defines `IRBuiltinRequirementKey* getBuiltinRequirementKey(IRIntegerValue kind)`. The sampled `source/slang/slang-ir.cpp` contains `IRBuilder::getPoison` but not `getBuiltinRequirementKey`. | Change the citation for `getBuiltinRequirementKey` to `source/slang/slang-ir-insts.h`, or split the sentence so only `getPoison` is attributed to `slang-ir.cpp`. |
