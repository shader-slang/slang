# Prompt: ir-reference/metadata.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/metadata.md` — the
per-opcode reference for IR metadata families that are *not*
decorations: `Layout` (around line 2617), `Attr` (around line 2650),
`Debug*` (around lines 2714-2750), and `SPIRVAsmOperand` (around
line 2754).

## Family-specific guidance

- Split `## Opcodes` into one sub-section per family:
  - **Layout** — `Layout` parent and its children
    (`VarLayout`, `EntryPointLayout`, `TypeLayout`,
    `StructLayout`, `OffsetAttr`, `SizeAttr`, ...).
  - **Attribute** — `Attr` parent and its children
    (general-purpose IR attributes that are *not* decorations).
  - **Debug info** — `DebugLine`, `DebugValue`, `DebugFunction`,
    `DebugScope`, `DebugBuildIdentifier`, `DebugCompilationUnit`,
    `DebugSource`, `DebugInlinedAt`, ... Cite the SPIR-V non-semantic
    debug info specification only where the opcode obviously maps
    to a SPIR-V instruction; do not over-claim.
  - **SPIR-V inline asm operands** — `SPIRVAsmOperand` family and
    related opcodes used by the inline `__intrinsic_asm` mechanism.
- The `AST origin` column for these opcodes is usually
  `(synthesized)` — they are introduced by IR passes
  (`slang-ir-insert-debug-value-store.cpp` for debug ops,
  `slang-ir-layout-impl.cpp` for layout, the SPIR-V emitter for
  asm operands). For `Layout` opcodes that originate from
  `[layout(...)]` attributes the origin is the corresponding
  `LayoutAttribute` (cite
  [../ast-reference/modifiers.md](../ast-reference/modifiers.md)).

## Notable opcodes

Cover at least:

- `Layout` (parent) — what it holds and how it relates to
  `LayoutDecoration` from [decorations.md](decorations.md).
- `VarLayout` — operand encoding for a single variable's layout.
- `DebugLine` — operand layout (file, line, column).
- `DebugScope` — how scopes nest for debug info.
- `SPIRVAsmOperand` — its role in carrying typed operands into
  inline SPIR-V assembly.

## Forbidden content

- Layout *algorithm* — see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and the
  `slang-ir-layout*.cpp` files.
- Debug-info *emission* — see
  [../pipeline/06-emit.md](../pipeline/06-emit.md) and
  `slang-emit-spirv.cpp`.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every leaf opcode under `Layout`, `Attr`, `Debug*`, and
      `SPIRVAsmOperand` is listed.
- [ ] Hierarchy diagram has four siblings (one per sub-family).
- [ ] At least one debug-info row cites the IR pass that introduces
      it.
