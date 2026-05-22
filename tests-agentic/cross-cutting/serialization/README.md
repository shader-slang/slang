---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-22T00:00:00+00:00
source_commit: 6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
watched_paths_digest: 0ae70d5983bffa93d300bdc22a2673cef60b90d8c3f77c446bf709e6c35e2e6a
source_doc: docs/llm-generated/cross-cutting/serialization.md
source_doc_digest: 53b3dea2b9cfe34f642561251b5ca2a9f9c25cccef2da3b136dd33fba9a90fd5
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/serialization

## Intent
Tests verify the serialization-machinery claims in
[`docs/llm-generated/cross-cutting/serialization.md`](../../../docs/llm-generated/cross-cutting/serialization.md)
that survive into the `slangc` CLI: `.slang-module` round-tripping
(write with `-o`, read with `-dump-module` / `-get-module-info`), the
`-get-supported-module-versions` probe that exposes the
backwards-compat version range, the `EmbeddedDownstreamIR` marker that
appears when `-embed-downstream-ir` is used, and the source-location
serialization that surfaces as `#line` directives in C-family text and
`OpSource` / `OpString` markers in SPIR-V assembly.

Most of the source doc is internal — the `serialize(serializer, value)`
template pattern, the Fossil memory-mappable property, the raw RIFF
chunk layout, the deprecated repro round-trip, and the
"adding a new serialized field" walkthrough are all out of scope
because they have no slangc-CLI observable. The bundle is therefore
small by design (9 tests).


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Container packaging composes AST + IR + auxiliary data; with -embed-downstream-ir a SPIR-V blob joins the container and surfaces as an EmbeddedDownstreamIR marker under -dump-module. | functional | [#riff-container-format](../../../docs/llm-generated/cross-cutting/serialization.md#riff-container-format) | [`embed-downstream-ir-spirv.slang`](embed-downstream-ir-spirv.slang) |
| Container packaging composes AST + IR + auxiliary data; with -embed-downstream-ir on -target dxil a DXIL blob joins the container and surfaces as an EmbeddedDownstreamIR(10 : Int, ...) marker under -dump-module. | functional | [#riff-container-format](../../../docs/llm-generated/cross-cutting/serialization.md#riff-container-format) | [`embed-downstream-ir-dxil.slang`](embed-downstream-ir-dxil.slang) |
| Negative control to the -embed-downstream-ir claim: without the flag, the container does not carry a downstream-IR chunk and -dump-module shows no EmbeddedDownstreamIR marker. | regression | [#riff-container-format](../../../docs/llm-generated/cross-cutting/serialization.md#riff-container-format) | [`no-embed-downstream-ir-absent.slang`](no-embed-downstream-ir-absent.slang) |
| Source locations survive the compile and reappear in emitted text as #line directives on C-family targets and as OpSource/OpLine markers on SPIR-V. | functional | [#source-location-serialization](../../../docs/llm-generated/cross-cutting/serialization.md#source-location-serialization) | [`source-loc-line-directives-multi-target.slang`](source-loc-line-directives-multi-target.slang) |
| When debug info is requested, source-location serialization expands to a debug-info stream that names the source file via an OpString in SPIR-V. | functional | [#source-location-serialization](../../../docs/llm-generated/cross-cutting/serialization.md#source-location-serialization) | [`source-loc-spirv-debug.slang`](source-loc-spirv-debug.slang) |
| A serialized module carries its name and version; -get-module-info reads them back, proving the version gate the doc names is recorded at write time. | functional | [#versioning-and-backwards-compatibility](../../../docs/llm-generated/cross-cutting/serialization.md#versioning-and-backwards-compatibility) | [`module-info-name-version.slang`](module-info-name-version.slang) |
| The compiler exposes the inclusive [min, max] range of module versions it accepts; -get-supported-module-versions prints the range that backwards-compat policy relies on. | functional | [#versioning-and-backwards-compatibility](../../../docs/llm-generated/cross-cutting/serialization.md#versioning-and-backwards-compatibility) | [`supported-module-versions.slang`](supported-module-versions.slang) |
| The version baked into a freshly-written module is a positive integer; the version-gate machinery the doc names always stamps a value. | functional | [#versioning-and-backwards-compatibility](../../../docs/llm-generated/cross-cutting/serialization.md#versioning-and-backwards-compatibility) | [`module-version-in-range.slang`](module-version-in-range.slang) |
| A .slang-module written by slangc can be re-read by the compiler and its IR dumped, demonstrating the AST/IR/container round-trip the doc describes. | functional | [#what-is-serialized](../../../docs/llm-generated/cross-cutting/serialization.md#what-is-serialized) | [`module-round-trip-dump.slang`](module-round-trip-dump.slang) |
| AST-serialization preserves the checked AST that backs an import-able module; the public symbol's export name and type signature survive the write/read cycle. | functional | [#what-is-serialized](../../../docs/llm-generated/cross-cutting/serialization.md#what-is-serialized) | [`module-roundtrip-preserves-public-symbol.slang`](module-roundtrip-preserves-public-symbol.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| The Fossil backend's memory-mappable property (`#fossil-backend`). | (unclassified) | [#fossil-backend](../../../docs/llm-generated/cross-cutting/serialization.md#fossil-backend) | an internal performance claim with no observable. |
| `Unrecognized`-opcode deserializer behaviour (`#versioning-and-backwards-compatibility`). | (unclassified) | [#versioning-and-backwards-compatibility](../../../docs/llm-generated/cross-cutting/serialization.md#versioning-and-backwards-compatibility) | would require constructing a module whose opcode set exceeds the current compiler's, which the CLI cannot do. |
| The `serialize(serializer, value)` template pattern (`#the-serialize-pattern`). | needs-unit-test | [#the-serialize-pattern](../../../docs/llm-generated/cross-cutting/serialization.md#the-serialize-pattern) | a C++ idiom, no CLI surface. |
| The `SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS` toggle (`#fossil-backend`). | compile-time-toggle | [#fossil-backend](../../../docs/llm-generated/cross-cutting/serialization.md#fossil-backend) | baked into the binary at compile time. |
| `-dump-repro` / `-load-repro` round-trip (`#round-trip-and-repro-files`). | deprecated | [#round-trip-and-repro-files](../../../docs/llm-generated/cross-cutting/serialization.md#round-trip-and-repro-files) | deprecated per CLAUDE.md. |
| The "Adding a new serialized field" developer walkthrough (`#adding-a-new-serialized-field`). | process-doc | [#adding-a-new-serialized-field](../../../docs/llm-generated/cross-cutting/serialization.md#adding-a-new-serialized-field) | process documentation. |
| Raw RIFF chunk-code inspection (`#riff-container-format`). | requires-external-tool | [#riff-container-format](../../../docs/llm-generated/cross-cutting/serialization.md#riff-container-format) | would require parsing bytes; the doc defers chunk codes to source files. |
| `-line-directive-mode source-map` zip emission (`#source-location-serialization`) — the FileCheck runner cannot see inside a zip archive without an external `unzip` step. The default `#line` mode covers the same user-facing round-trip claim. | requires-external-tool | [#source-location-serialization](../../../docs/llm-generated/cross-cutting/serialization.md#source-location-serialization) | Verification would need a non-Slang tool the runner does not include (e.g., `unzip`, hex dumper, decompiler). |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#what-is-serialized](../../../docs/llm-generated/cross-cutting/serialization.md#what-is-serialized) | undocumented-behavior | `## What is serialized` lists AST + IR + container as the three serialized flavors but does not name `-dump-module` or `-get-module-info` as the user-facing CLI commands that read each back. A "User-facing CLI" subsection naming `-dump-module` / `-get-module-info` / `-get-supported-module-versions` would let tests cite a CLI anchor instead of inferring from the prose. |  |
| [#riff-container-format](../../../docs/llm-generated/cross-cutting/serialization.md#riff-container-format) | undocumented-behavior | `## RIFF container format` describes container-level orchestration but never spells out the `EmbeddedDownstreamIR` chunk marker that `-dump-module` surfaces. The marker is observable in CLI output; the doc should note it as the user-visible name of the downstream-IR chunk. |  |
| [#versioning-and-backwards-compatibility](../../../docs/llm-generated/cross-cutting/serialization.md#versioning-and-backwards-compatibility) | undocumented-behavior | `## Versioning and backwards compatibility` says "the C++ side carries a versioning gate" but does not mention that the version integer is exposed via `slangc -get-module-info` and the supported range via `slangc -get-supported-module-versions`. |  |
| [#line](../../../docs/llm-generated/cross-cutting/serialization.md#line) | undocumented-behavior | `## Source-location serialization` describes round-tripping internal location records but does not enumerate the target-side shape of those locations (`#line` for C-family, `OpSource` / `OpString` / `OpLine` for SPIR-V, none for WGSL). Tests had to observe the per-target shape directly. |  |
| [#what-is-not-in-this-document](../../../docs/llm-generated/cross-cutting/serialization.md#what-is-not-in-this-document) | undocumented-behavior | The `## What is not in this document` section names `slang-fossil.h` for the fossil-chunk layout but the doc itself does not appear in the watched paths for this bundle, so even the format-level layout cannot be cross-referenced from inside a citation. |  |
