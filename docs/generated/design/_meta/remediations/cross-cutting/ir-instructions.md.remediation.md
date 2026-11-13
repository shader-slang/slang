---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:33:55Z
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary

All four findings were verified against the current source and fixed in the document. The critical padded-vector claim was corrected, the stable-name checker description and the `IRDecoration` link were fixed, and oversized Notes cells were condensed with the explanations moved to prose after their tables.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-ir-lower-buffer-element-type.cpp:2501 returns Natural layout for non-Khronos targets; natural vec3 float is size 12, align 4 | Removed "every other target keeps the padded vector"; HLSL keeps float3 spelling without implying 16-byte storage; SPIR-V ArrayStride 16 qualified as the default layout case |
| F-002 | fixed | extras/check-ir-stable-names.lua:229 treats extra entries as WARNING only | `check` now described as requiring a stable name per live opcode and unique IDs, retired entries allowed with a warning |
| F-003 | fixed | source/slang/slang-ir.h:140 only forward-declares; definition at source/slang/slang-ir-insts.h:35 | Definition link changed to slang-ir-insts.h |
| F-004 | fixed | Notes column must be one short phrase per the doc prompt | Condensed MetalPackedVec, bitCast, constexpr, makeVector, alloca, generic Notes; explanations moved to prose after each table (see also F-001) |
