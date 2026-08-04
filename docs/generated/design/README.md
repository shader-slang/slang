# LLM-Generated Architectural Documentation

This directory contains architectural documentation for the Slang compiler that
is **produced and refreshed by AI agents**, not authored by hand. It is a
deliberate experiment in keeping high-level design documentation in sync with
a fast-moving codebase by re-running agents instead of relying on humans to
update prose every time the source changes.

If the experiment succeeds, this tree is intended to eventually supersede the
overlapping handwritten documents in [../../design/](../../design). Until then,
both sets coexist and may be cross-referenced.

## What lives here

| Subtree | Purpose |
| --- | --- |
| [architecture/](architecture) | Logical decomposition of the source tree into subsystems, module groups, and their high-level dependencies |
| [pipeline/](pipeline) | End-to-end compilation flow: lexing, parsing, semantic checking, AST-to-IR lowering, pre-link mandatory passes (`04b`), per-target layout IR module (`04c`), post-link IR passes, code emission |
| [syntax-reference/](syntax-reference) | Slang surface syntax reverse-engineered from the lexer, parser, and core-module syntax declarations |
| [cross-cutting/](cross-cutting) | Concerns that span multiple pipeline stages: diagnostics, the IR instruction set, target backends, the core module, serialization |
| [ast-reference/](ast-reference) | Per-class reference for every concrete AST node (declarations, expressions, statements, types, modifiers, values, plus the abstract roots) |
| [ir-reference/](ir-reference) | Per-opcode reference for the Slang IR instruction set, grouped by family (types, values, control-flow, structure, generics/existentials, resources/atomics, differentiation, decorations, metadata, misc) |
| [name-resolution/](name-resolution) | Algorithmic rules for identifier scopes, lookup, shadowing, visibility, and overload resolution |
| [target-pipelines/](target-pipelines) | Ordered, control-flow-graph view of the IR passes that run for one target end-to-end; complements the unordered catalog in `pipeline/05-ir-passes.md`. Pages: SPIR-V, HLSL, Metal, WGSL, CUDA, plus an index. |
| [glossary.md](glossary.md) | Compiler-internals vocabulary used across this tree and the source; complements the user-facing [language-reference glossary](../../language-reference/glossary.md) |
| [_meta/](_meta) | Pipeline infrastructure: manifest, prompts, freshness state, regeneration driver |

## Trust model and freshness

Every generated `.md` file begins with a YAML front-matter block that records
how and when it was generated:

```yaml
---
generated: true
model: <model id>                     # identifier of the agent / model
generated_at: <ISO 8601 timestamp>
source_commit: <git sha>              # commit the agent saw
watched_paths_digest: <hex sha256>    # digest of the watched source slice
warning: "Auto-generated. May drift from source. Do not edit by hand."
---
```

The list of source paths each document is supposed to track lives in
[_meta/manifest.yaml](_meta/manifest.yaml). The driver in
[_meta/regenerate.py](_meta/regenerate.py) computes the current digest of
each document's watched paths and compares it against
[_meta/freshness.json](_meta/freshness.json) to decide whether the document
is **fresh** (the source has not changed since the last regeneration) or
**stale** (the source has changed and the document should be regenerated).

The trust ordering is:

1. The source code is always authoritative.
2. The handwritten documents in [../../design/](../../design) are second.
3. The documents in this tree are third — they should be plausible, but
   readers must assume drift is possible until they verify against the
   source.

## How to use these documents

Read top-down: start at [architecture/overview.md](architecture/overview.md)
to understand the major subsystems, then drill into either
[pipeline/](pipeline) (if you want to follow a compile from source code to
emitted target) or [cross-cutting/](cross-cutting) (if you want to
understand a concern that spans the whole compiler).

If you find an inaccuracy, **do not hand-edit the file**. Instead either:

- File it as a prompt-improvement task against the relevant prompt in
  [_meta/prompts/](_meta/prompts), or
- Improve the watched-paths list in [_meta/manifest.yaml](_meta/manifest.yaml)
  so the agent has the correct slice of source on its next run, and then
  regenerate.

See [_meta/regenerate.md](_meta/regenerate.md) for the operator workflow.

## Contract for generated files

The contract that every generated `.md` is expected to satisfy:

- Begins with the YAML front-matter block described above.
- Every workspace-relative path appearing in a markdown link
  (`[text](path)`) resolves to an existing file or directory at
  `source_commit`.
- Does not exceed the size cap configured for that document in the manifest
  (kept small enough that any single regeneration prompt is feasible to
  re-run end-to-end).
- Does not duplicate verbatim prose from [../../design/](../../design); it may
  cite those documents but should be original.
- Cites concrete file paths from `source/`, `prelude/`, and `include/` for
  any non-trivial claim about implementation.

The lint pass in `regenerate.py --lint` enforces the structural parts of this
contract. Style and content are evaluated by the operator running the
regeneration.
