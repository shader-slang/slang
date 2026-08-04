# Prompt: ir-reference/decorations.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/generated/design/ir-reference/decorations.md` — the
per-opcode reference for every entry in the `Decoration` family of
[../../../../source/slang/slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua)
(top-level `Decoration` entry around line 1591 through ~line 2476).

This is the largest single family in the Lua file. The page is
allowed to be long (size cap is generous), but it must remain a
*table-oriented* reference, not prose.

## Family-specific guidance

- Split `## Opcodes` into sub-tables grouped by the semantic role
  of the decorations:
  - **Naming and provenance** (`nameHint`, `highLevelDecl`, ...).
  - **Layout and binding** (`layout`, `LayoutDecoration`, `binding`,
    `register*`, `vkBinding`, `vkDescriptorSet`, ...).
  - **Loop and branch hints** (`branch`, `flatten`, `loopControl`,
    `loopMaxIters`, `loopExitPrimalValue`, ...).
  - **Target-specific definition and intrinsics**
    (`TargetSpecificDecoration` family: `target`, `targetIntrinsic`,
    `requirePrelude`, `glslOuterArray`, `intrinsicOp`,
    `intrinsicAsm`).
  - **Capability and availability**
    (`RequireCapabilityAtomDecoration`, capability-set decorations,
    `availabilityDecoration`).
  - **Interpolation and IO** (`interpolationMode`,
    `TargetSystemValue`, `vkSpecializationConstant`, ...).
  - **Differentiation markers** (`Differentiable`,
    `BackwardDifferentiable`, `NoDiffThis`, `PrimalSubst`,
    `DifferentialPairOverloadDecoration`, autodiff scope markers).
  - **Linkage and lifetime** (`KeepAliveDecoration`, `import`,
    `export`, `extern`, `public`, `linkage` flags).
  - **Symbol identity** (mangled names, hash-string keys, witness
    table identity).
  - **Catch-all** (every remaining decoration; bucket as "Other").
- The `AST origin` column for decorations should cite the AST
  modifier or attribute that lowers into the decoration. Many
  decorations come from `slang-check-modifier.cpp` and
  `slang-lower-to-ir.cpp`'s `applyModifiers*` helpers, or from
  the IR autodiff passes for the differentiation markers.
- The size cap is large but not unlimited. If the page risks
  exceeding the cap, leave at most one row per leaf decoration (no
  long prose), and rely on `## Notable opcodes` for context.

## Notable opcodes

Cover at least the following representative decorations:

- `nameHint` / `NameHintDecoration` — its role in preserving readable
  names across IR passes; cite that backends use it for output
  variable names.
- `layout` / `LayoutDecoration` — link to the
  [metadata.md](metadata.md) `Layout` family for the layout opcodes
  that hang off it.
- `targetIntrinsic` / `TargetIntrinsicDecoration` — operand layout
  (target capability set, definition string) and how each backend
  consumes it; cite that this is the mechanism for the prelude /
  core-module intrinsic declarations.
- `intrinsicOp` / `IntrinsicOpDecoration` — its single integer
  operand identifying a built-in opcode.
- `branch` / `flatten` / `loopControl` — control-flow hints that
  flow through to the backend.
- `KeepAliveDecoration` — why DCE skips marked symbols.
- `Differentiable` / `BackwardDifferentiable` — high-level autodiff
  intent markers, distinct from the autodiff opcodes themselves;
  cross-link to [differentiation.md](differentiation.md).

## Forbidden content

- AST-level modifiers themselves — see
  [../ast-reference/modifiers.md](../ast-reference/modifiers.md).
- Backend-specific consumption of decorations — see
  [../pipeline/06-emit.md](../pipeline/06-emit.md) and the
  `slang-emit-*.cpp` files.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every leaf opcode under the Lua `Decoration` entry is in one
      of the sub-tables.
- [ ] No row gives more than one short sentence in the `Summary`
      column.
- [ ] The page is under the manifest size cap; if not, drop
      decorative prose and consider folding rows.
