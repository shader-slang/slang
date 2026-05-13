# Prompt: ir-reference/control-flow.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/control-flow.md` — the
per-opcode reference for control-flow opcodes. The relevant Lua
ranges are the `block` entry (around line 829) and the
`TerminatorInst` group (around line 1291).

## Family-specific guidance

- Split `## Opcodes` into:
  - **Block and parameters** (`block` parent, `Param`).
  - **Branches** (`unconditionalBranch`, `conditionalBranch`,
    `loop`, `ifElse`, `Switch`).
  - **Function exits** (`Return`, `Unreachable`, `Discard`, `Throw`,
    `MissingReturn`).
  - **Other terminators** (`TryCall`, `GenericAsm`, ...).
- The Lua file may list additional non-terminator control-flow-related
  opcodes (e.g. `TryCall`, `GenericAsm`). Include them when they live
  in the same group; otherwise leave a brief note.
- The `AST origin` column should cite the
  [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
  visitors for statements: `lowerStmt`, `visitBlockStmt`,
  `visitIfStmt`, `visitForStmt`, `visitWhileStmt`, `visitDoWhileStmt`,
  `visitSwitchStmt`, `visitReturnStmt`, `visitBreakStmt`,
  `visitContinueStmt`, `visitDiscardStmt`. Note that `block` itself
  is a structural container; it does not have a single AST origin.

## Notable opcodes

Cover at least:

- `block` — parent flag, owning `Param` instructions, why blocks
  have no `phi` opcode (block parameters take that role; see the
  `block parameter` glossary entry).
- `Param` — its placement (always at the start of a block),
  contrast with SSA phi nodes.
- `loop` — operands (continue target, break target, body target),
  why the join (`break`) target is explicit; cite
  [../../design/ir.md](../../design/ir.md).
- `ifElse` — operands (condition, true target, false target, join
  target).
- `conditionalBranch` — why no operands beyond the targets are
  allowed (critical-edge prohibition).
- `Switch` — N-way fanout, the default case slot, per-case operand
  pairs.
- `Discard` — HLSL fragment discard semantics; only meaningful in
  fragment shaders.

## Forbidden content

- CFG construction algorithms — see
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).
- Dominator analysis and CFG-walking helpers — see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every Lua entry under `TerminatorInst` plus `block` and
      `Param` is in the `## Opcodes` tables.
- [ ] Hierarchy diagram shows `TerminatorInst` and its children.
- [ ] At least one row in each sub-table cites a
      `slang-lower-to-ir.cpp` visitor.
