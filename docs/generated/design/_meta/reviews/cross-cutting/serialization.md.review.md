---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:27:36+00:00
target_doc: cross-cutting/serialization.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: b2fdbecc7c8684a517087980212251808258fb9306248171e9069a3533bfec5b
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for cross-cutting/serialization.md

## Summary
The page covers the required serialization topics and its front matter and links look structurally valid. The main accuracy issue is that the source-location section says source contents and an expansion stack are serialized, while the watched source only records path/range/line metadata and reconstructs placeholder-sized files.

## Items checked
- Ran `regenerate.py show cross-cutting/serialization.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-serialize-ast.cpp`, `source/slang/slang-serialize-ast.h`, `source/slang/slang-serialize-container.cpp`, `source/slang/slang-serialize-container.h`, `source/slang/slang-serialize-fossil.cpp`, plus 11 more).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Reviewed the page against its per-document prompt and common generated-doc contract, including every required section and mention of every watched serialization file.
- Checked relative markdown links in the document body for workspace-relative shape and existing workspace targets.
- Spot-checked more than 10 source-backed claims and named symbols against the watched files, including `enumTypeNames`, `serializeEnum`, `FossilUInt`, `SerializationMode`, `SLANG_SCOPED_SERIALIZER_STRUCT`, `ISerializerImpl`, `SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS`, `RIFF::BuildCursor`, `SerialBinary::kModuleFourCC`, `writeSerializedModuleIR`, `traverseInstsInSerializationOrder`, `deserializeFromFlatModule`, `kMaxIRSerializationDepth`, `SerialSourceLocWriter`, and `SerialSourceLocReader`.
- Confirmed the page has no markdown source line-anchor citations requiring exhaustive line-anchor verification.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## The serialize() pattern`, lines 64-69 | The document names the read/write mode type as `SerializerMode`, but the watched header declares `SerializationMode`. This is a cited symbol mismatch in the central API summary. | `source/slang/slang-serialize.h:62` declares `enum class SerializationMode`; `source/slang/slang-serialize.h:122` uses `virtual SerializationMode getMode() = 0;`. | Replace `SerializerMode` with `SerializationMode`. |
| F-002 | major | `## Source-location serialization`, lines 200-207 | The document says the serializer captures `SourceFile` records including `path, content, expansion stack`. The watched source-location data structures and read path do not serialize file contents, and the reader reconstructs source files with only a size plus line/adjusted-line metadata. | `source/slang/slang-serialize-source-loc.h:74-84` defines `SourceInfo` as path index, range, line counts, and adjusted-line counts. `source/slang/slang-serialize-source-loc.cpp:141-165` writes the path string and line metadata, and `source/slang/slang-serialize-source-loc.cpp:270-277` reconstructs a file via `createSourceFileWithSize(...)` and one source view. | Rewrite the sentence to say it captures source-file path, source-location ranges, line starts, and adjusted line/file mappings, not source content or an expansion stack. |
| F-003 | minor | `### Fossil backend`, lines 126-131 | The document states that disabled Fossil validation is used for performance-critical paths such as loading the embedded core module, but the watched source only shows a hard-coded default macro and the alternate assertion branch; it does not show an embedded-core loading use or an override site. | `source/slang/slang-serialize-fossil.h:36-44` defines `SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS 1` and the enabled validation behavior. A workspace search found no override or embedded-core use in the watched serialization files. | Remove the embedded-core example or qualify the text to only describe the compile-time branches visible in `slang-serialize-fossil.h`. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
