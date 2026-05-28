# Prompt: cross-cutting/ir-instructions.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/cross-cutting/ir-instructions.md` — a
catalog of the Slang IR instruction set, derived from
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua).

This is a **generated reference**, not a tutorial. The deep design
rationale lives in [../../design/ir.md](../../design/ir.md) and
[../../design/ir-instruction-definition.md](../../design/ir-instruction-definition.md);
this document is an inventory.

Audience: a developer writing or modifying an IR pass who needs to
look up an opcode.

## Required structure

1. `# IR Instruction Catalog` (title)
2. `## Source` — state that the catalog is reverse-engineered from
   [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua); the
   header `slang-ir-insts.h` and the FIDDLE-generated enum
   `kIROp_*` in `build/source/slang/fiddle/slang-ir-insts-enum.h.fiddle`
   are downstream of that file.
3. `## Schema` — describe the structure of the Lua table (entries with
   names, optional `struct_name`, `operands`, `hoistable`,
   nested tables establishing inheritance hierarchies).
4. `## Instruction families` — group the instructions following the
   structure of the Lua file:
   - **Type instructions** (basic types, vector/matrix, array,
     resource types, struct types, generic types, ...)
   - **Value instructions** (literals, arithmetic, logical,
     comparisons, bitwise)
   - **Memory instructions** (load, store, address-of, var, alloca)
   - **Control-flow instructions** (branch, conditionalBranch,
     loop, switch, ifElse, return, unreachable, parameter)
   - **Function and module structure** (func, generic, block,
     module, global var)
   - **Specialization and existentials** (`Specialize`, `LookupWitness`,
     `ExtractExistentialValue`, ...)
   - **Decorations** (`*Decoration` opcodes)
   - **Resource and shader-IO opcodes** (binding decoration,
     interpolation modifiers, entry-point IR)
   For each family produce a table:

   ```markdown
   | Opcode | struct_name | Operands | Notes |
   | --- | --- | --- | --- |
   | `IntLit` | `IRIntLit` | (literal value, type) | Stored inline |
   ```

   The `Notes` column is one short phrase. You do not need to list
   every operand of every instruction; defer to the Lua source.
5. `## Hoistable / global / deduplicated values` — short paragraph
   citing [../../design/ir.md](../../design/ir.md) for the semantics.
6. `## Decorations` — note that some opcodes are conceptually
   decorations and the planned migration toward decorations-as-
   instructions; do not over-claim.

## Quality checklist (in addition to the universal one)

- [ ] Every opcode listed appears in
      [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua).
      Do not invent opcodes.
- [ ] Tables are summary-level: at most a few dozen entries per family,
      with explicit notes when truncating ("...plus ~N more in this
      family; see the Lua file for the full list").
- [ ] Document length under 48 KB.
