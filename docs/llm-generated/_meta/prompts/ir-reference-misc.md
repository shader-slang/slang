# Prompt: ir-reference/misc.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/misc.md` — the catch-all
per-opcode reference for opcodes that do not naturally fit any other
IR-reference family page.

This page exists to ensure the reference's coverage rule is satisfied
without forcing miscellaneous opcodes into mis-fitting categories.
Typical inhabitants include `Each`, `PackBranch`, `MakeWitnessPack`,
`getStringHash`, descriptor-heap and cooperative-vector helpers,
`GetPerVertexInputArray`, `ForceVarIntoStructTemporarilyBase`,
`BindingQuery`, `CastStorageToLogicalBase`, `IsType`, `CudaKernelLaunch`,
and similar.

## Family-specific guidance

- Use one `## Opcodes` table or a small number of sub-tables grouped
  by purpose (e.g. "pack / unpack helpers", "binding queries",
  "platform-launch", "value packs / each", "string hashing").
- For each opcode in this page, the `AST origin` is often either
  `(synthesized)` (an IR-pass artefact) or a specific lowering helper
  in
  [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp).
  Trace explicitly — do not guess.
- Cross-link to the sibling family pages whenever a misc opcode
  closely relates to one (e.g. `MakeWitnessPack` -> witness tables in
  [structure.md](structure.md)).

## Notable opcodes

Cover at least:

- `Each` — its iteration semantics in IR passes; cite the pass that
  introduces it.
- `PackBranch` — its three-operand encoding (pack, empty value,
  non-empty value) and what it pattern-matches.
- `MakeWitnessPack` — its role in passing witness tables through
  the value-pack mechanism.
- `getStringHash` — operand layout and stable-hash semantics.
- `CudaKernelLaunch` — operand encoding and its CUDA-specific
  emission.

## Forbidden content

- Anything that has a natural home in another `ir-reference/` page.
  Move the opcode there and update both prompts rather than letting
  it sit here.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] No opcode listed here also appears in another `ir-reference/`
      page's `## Opcodes` table.
- [ ] Every Lua entry in the file that has not been claimed by
      another family page is present here. If you find such an
      entry that you cannot place, list it with an explicit
      `(unclear)` summary.
- [ ] The page is small (cap is 32 KB); larger means an opcode has
      been mis-routed.
