# Prompt: ir-reference/differentiation.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/differentiation.md` — the
per-opcode reference for opcodes used by the Slang automatic-
differentiation passes.

The relevant Lua ranges include `MakeDifferentialPairBase`,
`DifferentialPairGetDifferentialBase`, and
`DifferentialPairGetPrimalBase` (around lines 900-931),
plus `ForwardDifferentiate`, `BackwardDifferentiate`,
`PrimalSubstitute`, `BackwardDiff*Context`, `RematFunc*`, and
related autodiff helpers.

## Family-specific guidance

- Split `## Opcodes` into:
  - **Differential-pair construction** (`MakeDiffPair`,
    `MakeDiffRefPair`).
  - **Differential-pair projections** (`GetDifferential`,
    `GetDifferentialPtr`, `GetPrimal`, `GetPrimalRef`).
  - **Differentiation operators** (`ForwardDifferentiate`,
    `BackwardDifferentiate`, `PrimalSubstitute`,
    `JVPDifferentiate`).
  - **Reverse-mode context** (`BackwardDiffPrimalContext`,
    `BackwardDiffPropagateContext`, `BackwardDiff*Context*`).
  - **Rematerialization** (`RematFunc*`, `checkpointObj`).
  - **Diagnostics and reporting** (`ReportCheckpointStore`,
    `DiffTypeInfo`).
- Most autodiff opcodes are `(synthesized)` — they are introduced
  by the autodiff IR passes
  (`slang-ir-autodiff*.cpp`,
  `slang-ir-differential-pair-overload-decoration.cpp`,
  ...). Cite that in the prose around the table and use `(synthesized)`
  in the `AST origin` column rather than guessing AST nodes.
- Cross-link to:
  - [types.md](types.md) for the differential-pair *types*.
  - [values.md](values.md) for ordinary make/extract patterns
    that this family mirrors.
  - [../../design/autodiff.md](../../design/autodiff.md) for the
    deep design rationale.

## Notable opcodes

Cover at least:

- `MakeDiffPair` — operand pair (primal, differential) and its
  result type.
- `GetDifferential` / `GetPrimal` — projections that reverse
  `MakeDiffPair`.
- `ForwardDifferentiate` — its operand is a function value to be
  differentiated.
- `BackwardDifferentiate` — pair with `ForwardDifferentiate`,
  produces a reverse-mode adjoint function.
- `BackwardDiffPropagateContext` — its role in plumbing the
  reverse-mode state between basic blocks.
- `checkpointObj` / `ReportCheckpointStore` — how the
  rematerialization pipeline records checkpoint values.

## Forbidden content

- The autodiff algorithm itself — see
  [../../design/autodiff.md](../../design/autodiff.md) and
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
- The user-facing `[Differentiable]` syntax — see
  [../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every Lua entry inside the three `*Base` groups is listed.
- [ ] Every `(synthesized)` row is consistent with the autodiff pass
      that introduces it (cite the pass file name in the family-level
      prose).
- [ ] Hierarchy diagram shows the three `*Base` groups.
