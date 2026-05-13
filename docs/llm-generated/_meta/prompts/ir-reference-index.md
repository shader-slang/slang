# Prompt: ir-reference/index.md

See [_common.md](_common.md) for the universal rules. This page is the
navigation entry point for the IR reference subtree; the per-page
**IR-reference family contract** in `_common.md` does not directly
apply.

## Target

Produce `docs/llm-generated/ir-reference/index.md` — a short
navigation document that introduces the IR reference subtree, names
the families and their Lua entry ranges, and links every family page.

Audience: a developer who just opened
`docs/llm-generated/ir-reference/` and needs to pick the right page.

## Required structure

1. `# IR Reference` title.
2. Two-paragraph intro: what this subtree is, who it is for, and how
   it relates to the conventions page
   [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
   (schema, flag bits, module versioning, the "add an opcode" workflow)
   and the lowering pipeline page
   [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) (when
   AST nodes lower to IR).
3. `## Family taxonomy` — a mermaid `flowchart TD` rooted at
   `IRInst` that shows each family page as a leaf. Group siblings so
   the picture stays legible (don't put every page directly under
   `IRInst`).
4. `## Pages` — a table:

   ```markdown
   | Page | Family | Lua entry root | Approx. opcodes |
   | --- | --- | --- | --- |
   | [types.md](types.md) | Type instructions | `Type` (line ~20) | ~200 |
   ```

   Counts are approximate: count `struct_name = "..."` entries plus
   bare opcode entries that fall into the family in
   [../../../source/slang/slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua).
   Round to the nearest ten and prefix with `~`.

5. `## How AST nodes lower to IR` — three or four sentences pointing
   the reader at the `slang-lower-to-ir.cpp` `visit*` functions for
   the AST origin information, and at the
   [../ast-reference/](../ast-reference/) subtree for the AST side.
   Note that opcodes with no direct AST source (autodiff
   intermediates, IR-pass artefacts) carry `(synthesized)` or `—` in
   the per-page AST-origin column.
6. `## Cross-cutting topics` — bullets pointing at the
   `docs/llm-generated/pipeline/` and
   `docs/llm-generated/cross-cutting/` documents that touch the IR:
   AST-to-IR lowering (04), IR passes (05), code emission (06),
   serialization, target backends, and the glossary entries for
   IR-related terms.
7. `## How to navigate` — 3-4 sentence guidance: when to start at
   [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
   for conventions versus jumping straight into a family page; how
   the AST-origin column works; that abstract intermediate Lua entries
   only appear in the `## Family hierarchy` diagrams, never in the
   `## Opcodes` tables.

## Quality checklist (in addition to the universal one)

- [ ] Every link to a family page resolves at the current commit.
- [ ] Approximate counts are within +/- 10 of the actual opcode count
      for that family in the Lua file.
- [ ] The mermaid diagram covers every family page exactly once.
- [ ] No opcode from a family page is described in detail here — this
      is a navigation page, not a reference page.
