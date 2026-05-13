# Prompt: cross-cutting/serialization.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/cross-cutting/serialization.md` — a
description of the AST and IR serialization machinery: what gets
serialized, in which format, and how round-tripping works.

Audience: a developer adding a serialized field, debugging a
deserialization failure, or working on cross-version stability of IR
modules.

## Required structure

1. `# Serialization` (title)
2. `## What is serialized` — three flavors visible in the watched
   paths:
   - **AST modules** (handled by
     [slang-serialize-ast.cpp](../../../source/slang/slang-serialize-ast.cpp)).
   - **IR modules** (handled by
     [slang-serialize-ir.cpp](../../../source/slang/slang-serialize-ir.cpp)
     plus
     [slang-serialize-ir-types.cpp](../../../source/slang/slang-serialize-ir-types.cpp)).
   - **Containers** (RIFF-based packaging via
     [slang-serialize-container.cpp](../../../source/slang/slang-serialize-container.cpp),
     [slang-serialize-riff.cpp](../../../source/slang/slang-serialize-riff.cpp)).
3. `## Backends` — describe the two encoding backends present:
   - **Generic serialize** ([slang-serialize.cpp](../../../source/slang/slang-serialize.cpp))
   - **Fossil** ([slang-serialize-fossil.cpp](../../../source/slang/slang-serialize-fossil.cpp))
   Identify what each is used for (verify by reading the watched
   files).
4. `## RIFF container format` — explain at a high level that modules
   are wrapped in a RIFF-style chunked container. Cite
   [slang-serialize-riff.cpp](../../../source/slang/slang-serialize-riff.cpp).
5. `## Source-location serialization` — point at
   [slang-serialize-source-loc.cpp](../../../source/slang/slang-serialize-source-loc.cpp).
6. `## Versioning and backwards compatibility` — link
   [../../design/backwards-compat-for-ir-modules.md](../../design/backwards-compat-for-ir-modules.md)
   as the authoritative document. State the constraint that
   `slang-ir-insts.lua` opcode insertions need to obey for older
   modules to keep deserializing.
7. `## Round-trip and repro files` — short note that the historical
   `-dump-repro` / `-load-repro` machinery is deprecated (per
   [CLAUDE.md](../../../CLAUDE.md)), so do not document it as
   recommended; mention it only if it appears in the watched paths
   so readers know it exists.

## Quality checklist (in addition to the universal one)

- [ ] Every serialize-related file in the watched paths is mentioned.
- [ ] No invented format identifiers or chunk types.
- [ ] Document length under 24 KB.
