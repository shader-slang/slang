---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:14:10Z
source_commit: ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
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
small by design (8 tests).

## Claims enumerated

| Claim ID | Anchor                                       | Claim (one line)                                                                                                       | Tests                                                       |
| -------- | -------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------- |
| C-01     | `#what-is-serialized`                        | A `.slang-module` written by slangc can be re-read and its IR dumped, demonstrating the AST/IR/container round-trip.   | [`module-round-trip-dump.slang`](module-round-trip-dump.slang)                              |
| C-02     | `#what-is-serialized`                        | AST serialization preserves the checked AST of a public symbol — its mangled export name and signature survive.        | [`module-roundtrip-preserves-public-symbol.slang`](module-roundtrip-preserves-public-symbol.slang)            |
| C-03     | `#versioning-and-backwards-compatibility`    | A serialized module carries its module name and module-version integer; `-get-module-info` reads them back.            | [`module-info-name-version.slang`](module-info-name-version.slang)                            |
| C-04     | `#versioning-and-backwards-compatibility`    | The compiler exposes the inclusive `[min, max]` range of module versions it accepts via `-get-supported-module-versions`. | [`supported-module-versions.slang`](supported-module-versions.slang)                           |
| C-05     | `#versioning-and-backwards-compatibility`    | Every module written by this build stamps a positive integer version (the version-gate plumbing always writes a value). | [`module-version-in-range.slang`](module-version-in-range.slang)                             |
| C-06     | `#riff-container-format`                     | With `-embed-downstream-ir`, the container gains a downstream-IR chunk surfacing as `EmbeddedDownstreamIR(...)` under `-dump-module`. | [`embed-downstream-ir-spirv.slang`](embed-downstream-ir-spirv.slang)                           |
| C-07     | `#riff-container-format`                     | Without `-embed-downstream-ir`, the container omits the downstream-IR chunk and no `EmbeddedDownstreamIR` marker appears. | [`no-embed-downstream-ir-absent.slang`](no-embed-downstream-ir-absent.slang)                       |
| C-08     | `#source-location-serialization`             | Source locations survive compilation to text as `#line` directives on C-family targets and `OpSource` on SPIR-V.       | [`source-loc-line-directives-multi-target.slang`](source-loc-line-directives-multi-target.slang)             |
| C-09     | `#source-location-serialization`             | When `-g2` is set, the SPIR-V debug-info stream carries an `OpString` naming the source file.                          | [`source-loc-spirv-debug.slang`](source-loc-spirv-debug.slang)                              |

## Tests in this bundle

| File                                              | Intent     | Doc anchor                                |
| ------------------------------------------------- | ---------- | ----------------------------------------- |
| [`embed-downstream-ir-spirv.slang`](embed-downstream-ir-spirv.slang)                 | functional | `#riff-container-format`                  |
| [`module-info-name-version.slang`](module-info-name-version.slang)                  | functional | `#versioning-and-backwards-compatibility` |
| [`module-round-trip-dump.slang`](module-round-trip-dump.slang)                    | functional | `#what-is-serialized`                     |
| [`module-roundtrip-preserves-public-symbol.slang`](module-roundtrip-preserves-public-symbol.slang)  | functional | `#what-is-serialized`                     |
| [`module-version-in-range.slang`](module-version-in-range.slang)                   | functional | `#versioning-and-backwards-compatibility` |
| [`no-embed-downstream-ir-absent.slang`](no-embed-downstream-ir-absent.slang)             | regression | `#riff-container-format`                  |
| [`source-loc-line-directives-multi-target.slang`](source-loc-line-directives-multi-target.slang)   | functional | `#source-location-serialization`          |
| [`source-loc-spirv-debug.slang`](source-loc-spirv-debug.slang)                    | functional | `#source-location-serialization`          |
| [`supported-module-versions.slang`](supported-module-versions.slang)                 | functional | `#versioning-and-backwards-compatibility` |

## Doc gaps observed

- `## What is serialized` lists AST + IR + container as the three
  serialized flavors but does not name `-dump-module` or
  `-get-module-info` as the user-facing CLI commands that read each
  back. A "User-facing CLI" subsection naming
  `-dump-module` / `-get-module-info` / `-get-supported-module-versions`
  would let tests cite a CLI anchor instead of inferring from the
  prose.
- `## RIFF container format` describes container-level orchestration
  but never spells out the `EmbeddedDownstreamIR` chunk marker that
  `-dump-module` surfaces. The marker is observable in CLI output;
  the doc should note it as the user-visible name of the
  downstream-IR chunk.
- `## Versioning and backwards compatibility` says "the C++ side
  carries a versioning gate" but does not mention that the version
  integer is exposed via `slangc -get-module-info` and the supported
  range via `slangc -get-supported-module-versions`.
- `## Source-location serialization` describes round-tripping
  internal location records but does not enumerate the target-side
  shape of those locations (`#line` for C-family, `OpSource` /
  `OpString` / `OpLine` for SPIR-V, none for WGSL). Tests had to
  observe the per-target shape directly.
- The `## What is not in this document` section names `slang-fossil.h`
  for the fossil-chunk layout but the doc itself does not appear in
  the watched paths for this bundle, so even the format-level layout
  cannot be cross-referenced from inside a citation.

## Out of scope (no-GPU runner)

- The `serialize(serializer, value)` template pattern
  (`#the-serialize-pattern`) — a C++ idiom, no CLI surface.
- The Fossil backend's memory-mappable property (`#fossil-backend`)
  — an internal performance claim with no observable.
- The `SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS` toggle
  (`#fossil-backend`) — baked into the binary at compile time.
- Raw RIFF chunk-code inspection (`#riff-container-format`) — would
  require parsing bytes; the doc defers chunk codes to source files.
- `-dump-repro` / `-load-repro` round-trip
  (`#round-trip-and-repro-files`) — deprecated per CLAUDE.md.
- The "Adding a new serialized field" developer walkthrough
  (`#adding-a-new-serialized-field`) — process documentation.
- `Unrecognized`-opcode deserializer behaviour
  (`#versioning-and-backwards-compatibility`) — would require
  constructing a module whose opcode set exceeds the current
  compiler's, which the CLI cannot do.
- `-line-directive-mode source-map` zip emission
  (`#source-location-serialization`) — the FileCheck runner cannot
  see inside a zip archive without an external `unzip` step. The
  default `#line` mode covers the same user-facing round-trip
  claim.
- DXIL `-embed-downstream-ir` — requires `dxc.exe`, not available
  on the no-GPU runner used here. SPIR-V embedding covers the
  cross-cutting "container packages a downstream-IR blob" claim.
