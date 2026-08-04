---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:20Z
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/module-map.md

## Summary
Both findings were valid and in-scope: the per-doc prompt requires a mechanical, exhaustive decomposition of the watched `source/*/*.{h,cpp}` files, and two file families were absent from the map. I fixed both by adding compact rows/subgroups for the omitted logical units, every cited file confirmed present in the resolved watched-path set. The document grew from 20.6 KB to 22.4 KB, still under the 32 KB cap.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/compiler-core/slang-json-{parser,value,native,rpc,rpc-connection,source-map-util}.*`, `slang-language-server-protocol.*`, `slang-rich-diagnostics-render.*`, `slang-source-map.*`, `slang-nvrtc-compiler.*`, and `slang-llvm-compiler.*` are all in the resolved watched paths but unrepresented. | Added compiler-core rows for JSON tokenizer/parser, JSON value model, JSON-RPC, LSP protocol types, rich diagnostic rendering, and source maps; folded NVRTC/LLVM into the per-vendor compilers row and dropped the stray `slang-json-lexer.cpp` from it. |
| F-002 | fixed | `source/slang/slang-language-server*.{h,cpp}` and `slang-reflection-api.cpp` / `slang-reflection-json.*` are watched but had no row; confirmed at `source/slang/slang-language-server.cpp:3` and `source/slang/slang-reflection-api.cpp:27`. | Added a `### Reflection API` subsection (2 rows) and a `### Language server` subsection (server core plus per-feature helpers) under `source/slang/`. |
