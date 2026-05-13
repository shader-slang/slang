# Prompt: ast-reference/modifiers.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract**.

## Target

Produce `docs/llm-generated/ast-reference/modifiers.md` — the per-node
reference for the `Modifier` family declared in
[slang-ast-modifier.h](../../../source/slang/slang-ast-modifier.h).

## Family-specific guidance

- The hierarchy diagram should group modifiers into the abstract
  intermediates the header introduces, e.g. `Modifier`,
  `SyntaxModifier`-like groupings, `Attribute`-rooted attributes,
  HLSL-compat attributes, GLSL/Vulkan-mapping modifiers, target
  intrinsics modifiers, layout modifiers.
- The `## Nodes` table must cover every concrete class in
  `slang-ast-modifier.h`.
- Grammar column: parsed modifiers and attributes link to
  [../syntax-reference/grammar.md#modifiers](../syntax-reference/grammar.md#modifiers)
  or `#attributes` as appropriate. Modifiers produced only by
  synthesis (e.g. `IntrinsicOpModifier`, internal HLSL legalization
  modifiers) use `(none)`.

## Notable nodes

Cover at least the following:

- The distinction between **modifiers** (keyword-like, no parameter
  list — `in`, `out`, `inout`, `const`, `uniform`, `static`,
  `volatile`) and **attributes** (`[attr(args)]` syntax that subclass
  `Attribute`).
- `HLSLAttribute` family — its role as a common ancestor for HLSL
  shader-stage attributes.
- `IntrinsicOpModifier` — how the core module uses it to bind a
  function to an IR opcode; see
  [../cross-cutting/core-module.md](../cross-cutting/core-module.md).
- `TargetIntrinsicModifier` — how it ties a declaration to a target
  backend.
- `LayoutModifier` / `HLSLLayoutModifier` family — their role in
  parameter binding.
- Visibility / linkage modifiers (`PublicModifier`, `PrivateModifier`,
  `InternalModifier`).
- `RequireCapabilityAttribute` — its relationship to the capability
  system in [../cross-cutting/targets.md](../cross-cutting/targets.md).
- The `GLSLLayout*Modifier` family — how Slang represents GLSL-style
  layout qualifiers.

## Forbidden content

- IR decorations — those live in
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- Semantic semantics (no pun) of modifiers — checked in
  [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete class in `slang-ast-modifier.h` appears in the
      table.
- [ ] The distinction between modifiers and attributes is explicit in
      the introduction.
- [ ] Grammar links resolve.
