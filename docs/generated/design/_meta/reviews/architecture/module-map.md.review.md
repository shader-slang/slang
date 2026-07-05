---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:46+00:00
target_doc: architecture/module-map.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 3fedec1d8eadc2bf2e0fb417739cd43fb662af67bb19d9334c40fa0168b98cc4
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for architecture/module-map.md

## Summary
The included rows I spot-checked are generally accurate, but the document does not meet its prompt's exhaustive lookup-table contract. Two major watched file families are missing from the map: several `source/compiler-core/` infrastructure families and multiple `source/slang/` families such as the language server and reflection API.

## Items checked
- Ran `regenerate.py show architecture/module-map.md` and reviewed the target document, `_common.md`, `architecture-module-map.md`, the dependency document `architecture/overview.md`, and representative resolved watched source files from `prelude/`, `source/core/`, `source/compiler-core/`, `source/slang/`, `source/slangc/`, and peer `source/*` directories.
- Checked front matter for all required keys, the recorded target source commit, the warning string, and a 64-character hex watched-path digest copied from the target document.
- Spot-checked more than 10 concrete included claims against source files, including claims about `slang-lexer`, `slang-token`, `SourceManager`, `DiagnosticSink`, `slang-parser`, `slang-preprocessor`, AST families, `slang-lower-to-ir`, `slang-ir-insts.lua`, `slang-emit-hlsl`, and `source/slangc/main.cpp`.
- Checked representative omitted watched files against the prompt's "mechanical, exhaustive decomposition" requirement and the quality checklist requiring table paths to come from watched paths.
- Checked the visible relative links used for generated peer documents and representative workspace files; no dangling relative links were found in the checked set.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## source/compiler-core/ — language-agnostic compiler infrastructure`, lines 56-70 | The compiler-core table is not exhaustive. It includes lexer, diagnostics, artifacts, downstream compiler glue, include search, JSON lexer, and command-line args, but omits watched logical units for JSON parsing/value/RPC, language-server protocol types, rich diagnostic rendering, source maps, and several downstream compiler adapters. | `source/compiler-core/slang-json-parser.cpp:14` defines `JSONParser::_parseObject`; `source/compiler-core/slang-json-rpc-connection.cpp:19` defines `JSONRPCConnection::init`; `source/compiler-core/slang-language-server-protocol.h:13` opens the `LanguageServerProtocol` namespace; `source/compiler-core/slang-rich-diagnostics-render.cpp:33-37` describes diagnostic layout and rendering. | Add compact rows for the omitted compiler-core families, grouping related files where appropriate, for example JSON parsing/value/RPC, language-server protocol, rich diagnostic rendering, source maps, and remaining downstream compiler adapters. |
| F-002 | major | `## source/slang/ — frontend, IR, passes, emit`, lines 72-235 | The `source/slang/` section omits major watched file families, so the page is not the promised file-level lookup table. In particular, there is no row or subgroup for the Slang language server family and no row for the reflection API implementation. | `source/slang/slang-language-server.cpp:3` says the file implements the Slang language server and includes the completion, document-symbol, inlay-hint, semantic-token, and AST-lookup helpers at `source/slang/slang-language-server.cpp:20-24`; `source/slang/slang-reflection-api.cpp:27` starts conversion routines for the strongly typed reflection API. | Add rows or a short subgroup for `slang-language-server*` and `slang-reflection-*` / reflection API files, and sweep the remaining watched `source/slang/*.h` and `*.cpp` files for similar omitted families before marking the map complete. |

## No-issues notes
- The front matter is structurally valid and uses the target document's own source commit and watched-path digest.
- The large `slang-ir-*` pass family is appropriately summarized by category rather than enumerating hundreds of pass files, which matches the per-doc prompt.
- The `source/slang-record-replay/` and `source/slang-llvm/` sections correctly identify those peer directories as separate logical unit groups from the main `source/slang/` target.
