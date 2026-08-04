# Prompt: architecture/module-map.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/architecture/module-map.md` — a
mechanical, exhaustive decomposition of `source/` and `prelude/` into
**logical units** (a single C++ class hierarchy or a tightly-coupled
group of files), grouped into **logical unit groups** (subsystems
introduced in [overview.md](overview.md)).

The output is intended to be the "look-up table" companion to
[overview.md](overview.md): given a feature you want to work on, find
the right logical unit and the files that constitute it.

Audience: a developer who has read [overview.md](overview.md) and now
wants to find, for example, "where is the AST builder defined?" or
"which files implement IR linking?".

## Required structure

For each logical unit group (use the same names as in
[overview.md](overview.md)), produce a level-2 heading
(`## source/slang/ — frontend, IR, passes, emit`, etc.) followed by a
table:

```markdown
| Logical unit | Files | Responsibility |
| --- | --- | --- |
| `Lexer` | [slang-lexer.{h,cpp}](../../../../source/compiler-core/slang-lexer.cpp) | Tokenizes a source buffer into a stream of `Token` |
| ... | ... | ... |
```

For source/slang/ (which is large), produce a sub-grouping by prefix:

- `slang-ast-*` — AST node hierarchy (one logical unit per file family,
  e.g. expressions, statements, types, modifiers, decls).
- `slang-parser*`, `slang-preprocessor*` — frontend.
- `slang-check*` — semantic checking (one logical unit per file).
- `slang-lower-to-ir*` — AST-to-IR lowering.
- `slang-ir.*`, `slang-ir-insts.*` — IR core.
- `slang-ir-*` (excluding the core above) — IR passes; **do not enumerate
  every pass here** (that belongs in
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)). Instead
  describe pass categories and point at the pipeline doc.
- `slang-emit*` — code emission (one row per backend: HLSL, GLSL, SPIR-V,
  Metal, WGSL, CPP, CUDA, Torch, VM, plus the shared `slang-emit-c-like`
  base).
- `slang-serialize*` — IR/AST/RIFF serialization (point at
  [../cross-cutting/serialization.md](../cross-cutting/serialization.md)).
- `slang-diagnostics*`, `diagnostics/*.lua` — diagnostics
  (point at [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).
- `slang-capability*`, `slang-capabilities.capdef` — capability system.
- `slang-module*`, `slang-compile-request*`, `slang-linkage*` — top-level
  objects already introduced in [overview.md](overview.md).

## Quality checklist (in addition to the universal one)

- [ ] Every file path in the tables exists in the watched paths.
- [ ] Each row's "Responsibility" column is at most one sentence.
- [ ] No row describes a single function or a class member; only
      file-or-cluster-scale logical units.
- [ ] Document length under 32 KB. If you are running over, prefer
      summary-with-link rows over enumerating every header.
- [ ] Every logical unit group in [overview.md](overview.md) has its own
      level-2 section here.
