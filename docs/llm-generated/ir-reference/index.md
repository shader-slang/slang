---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-15T15:55:00+00:00
source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
watched_paths_digest: 44cc076396b4503f18997a6be579c3163209ab7a24f87f3aae489b26a3963cbd
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# IR Reference

This subtree of `docs/llm-generated/` is a per-family reference for the
Slang Intermediate Representation. Every concrete opcode declared in
[../../../source/slang/slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua)
appears in a family page below, tabulated with its C++ wrapper
struct (from [../../../source/slang/slang-ir-insts.h](../../../source/slang/slang-ir-insts.h)),
its operand shape, its op-flags (`H` hoistable, `P` parent, `G` global),
the AST node(s) that lower into it (sourced from
[../../../source/slang/slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)),
and a one-line summary. Notable opcodes that carry semantics a table row
cannot convey have short call-outs further down each page.

Most opcodes appear in exactly one family page. A small number of
opcodes that play two distinct roles (e.g. `struct` / `class` /
`interface` as both types and parent containers, `param` as both a
block parameter and a function parameter, `global_var` as both a
module-scope structural slot and a value-producing op) appear in two
pages — once as the primary listing and once as a cross-link row
that points back to the canonical page. Abstract / grouping-only
Lua entries (`Type`, `Constant`, `TerminatorInst`, `BasicType`,
`MakeDifferentialPairBase`, `CastStorageToLogicalBase`,
`LiveRangeMarker`, `SemanticAttr`, `LayoutResourceInfoAttr`,
`SPIRVAsmOperand`, `Undefined`, `BindingQuery`, ...) appear only in
hierarchy diagrams, never in opcode tables.

The family pages are intentionally narrow: they describe *shape and
provenance*, not *behaviour of the passes that consume the IR*. For
the conventions every opcode obeys (schema, flag bits, hoistable/global
deduplication, module versioning, the workflow for adding a new opcode),
see [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
For when AST nodes lower to IR and which lowering helpers run, see
[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md). For what the
IR passes do afterwards, see
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

## Family taxonomy

```mermaid
flowchart TD
  IRInst --> Types["Types"]
  IRInst --> Values["Values"]
  IRInst --> Structure["Structure"]
  IRInst --> ControlFlow["Control flow"]
  IRInst --> GenericsExistentials["Generics &amp; existentials"]
  IRInst --> ResourcesAtomics["Resources &amp; atomics"]
  IRInst --> Differentiation["Differentiation"]
  IRInst --> Decoration["Decoration"]
  IRInst --> Metadata["Metadata"]
  IRInst --> Misc["Misc"]
  Types --> types_md["types.md"]
  Values --> values_md["values.md"]
  Structure --> structure_md["structure.md"]
  ControlFlow --> controlflow_md["control-flow.md"]
  GenericsExistentials --> generics_md["generics-and-existentials.md"]
  ResourcesAtomics --> resources_md["resources-and-atomics.md"]
  Differentiation --> differentiation_md["differentiation.md"]
  Decoration --> decorations_md["decorations.md"]
  Metadata --> metadata_md["metadata.md"]
  Misc --> misc_md["misc.md"]
```

## Pages

| Page | Family | Lua entry root | Approx. opcodes |
| --- | --- | --- | --- |
| [types.md](types.md) | Type instructions | `Type` (line ~20) | ~160 |
| [values.md](values.md) | Constants, arithmetic, conversions, memory, aggregate constructors, constexpr arithmetic/casts, string and native-pointer helpers | `Constant` (line ~838) and top-level value opcodes; constexpr arithmetic cluster ~line 3142 | ~150 |
| [structure.md](structure.md) | Module structure: functions, generics, globals, structs, interfaces, witness tables | `GlobalValueWithCode` (line ~787), `module` (line ~827) | ~20 |
| [control-flow.md](control-flow.md) | Block, parameters, branches, function exits, target / quad-execution `Require*` markers | `TerminatorInst` (line ~1294) + `block` / `Param` at top level | ~25 |
| [generics-and-existentials.md](generics-and-existentials.md) | `specialize`, witness lookup, existential pack/unpack, RTTI, type-flow specialization (sets, tagged unions, dispatchers) | Top-level (e.g. `specialize` ~line 932, `lookupWitness` ~line 933); type-flow cluster ~line 2880 | ~55 |
| [resources-and-atomics.md](resources-and-atomics.md) | Image/buffer/sampler ops, shader IO, atomics, barriers, fragment-shader interlocks, cooperative matrix/vector, wave intrinsics, raytracing | `AtomicOperation` (line ~1071) + top-level resource opcodes | ~85 |
| [differentiation.md](differentiation.md) | Autodiff: differential pairs, forward/backward differentiate, reverse-mode contexts, autodiff temporaries, `DiffTypeInfo` | `MakeDifferentialPairBase` (line ~901) + top-level autodiff opcodes | ~35 |
| [decorations.md](decorations.md) | Decoration family (metadata attached to instructions) | `Decoration` (line ~1594) | ~180 |
| [metadata.md](metadata.md) | `Layout`, `Attr`, `Debug*`, `SPIRVAsmOperand` | `Layout` (line ~2619), `Attr` (line ~2652), `Debug*` (line ~2716), `SPIRVAsmOperand` (line ~2756) | ~55 |
| [misc.md](misc.md) | System opcodes (`nop`, `Unrecognized`), pack/expansion, type queries, size/alignment, storage casts, liveness markers, descriptor heaps, tensor / runtime helpers, kernel launch | Top-level miscellaneous opcodes | ~55 |

Counts are approximate, rounded to the nearest ten at the
`source_commit` recorded in this file's front-matter. They count
`struct_name = "..."` entries plus bare opcode entries that fall into
the family in
[../../../source/slang/slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua).
The exact count drifts as opcodes are added, removed, or moved between
families; the regeneration pipeline surfaces mismatches as staleness.

## How AST nodes lower to IR

The "AST origin" column on every family page identifies which AST node
classes lower into a given opcode. Those mappings come from the ~165
`visit*` member functions in
[../../../source/slang/slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
(for example, `visitVarDecl` emits `Var`, `visitInfixExpr` dispatches
to arithmetic opcodes such as `Add`, `Sub`, `Mul`). For the AST side
of that mapping see the [../ast-reference/](../ast-reference/) subtree,
in particular [../ast-reference/expressions.md](../ast-reference/expressions.md),
[../ast-reference/statements.md](../ast-reference/statements.md), and
[../ast-reference/declarations.md](../ast-reference/declarations.md).

Many opcodes have no direct AST source: they are produced by IR
passes (specialization, autodiff, generics legalization, address-space
inference, SPIR-V emit fix-ups) and may even be retired before code
emission. Such opcodes carry `(synthesized)` or `—` in the AST-origin
column on each family page.

## Cross-cutting topics

- [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) —
  AST-to-IR lowering pipeline; how `IRBuilder`, `IRGenContext`, and
  the `visit*` methods translate AST into IR.
- [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) —
  the IR passes that legalize, specialize, and optimize the IR.
- [../pipeline/06-emit.md](../pipeline/06-emit.md) — how target
  emitters consume legalized IR.
- [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
  — IR schema, op-flag conventions, hoistable/global deduplication,
  module versioning, and the workflow for adding a new opcode.
- [../cross-cutting/serialization.md](../cross-cutting/serialization.md)
  — how IR modules are serialized.
- [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)
  — IR instructions carry `SourceLoc`s through the diagnostic system.
- [../glossary.md](../glossary.md) — definitions of `IRInst`, `IROp`,
  `IRBuilder`, `IRModule`, `parent instruction`, `terminator
  instruction`, `block parameter`, `decoration`, `hoistable
  instruction`, `target intrinsic`, `differential pair`, `witness
  table`, `existential type`, `specialization`, `single static
  assignment (SSA)`.

## How to navigate

Start at [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
if you are new to the IR: it covers schema, op-flag bits, and module
versioning that every family page below assumes. Then jump straight
to the family page for the opcode you care about. Each family page
opens with a `## Source` paragraph that links to the relevant Lua
entry range and to the `slang-lower-to-ir.cpp` visitors that produce
opcodes in that family.

Within a family page, the `## Opcodes` table is the canonical index.
Abstract / grouping intermediate Lua entries (such as `BasicType`,
`MakeDifferentialPairBase`, `TerminatorInst`, the abstract `Decoration`
root, plus the smaller parents `Undefined`, `BindingQuery`,
`CastStorageToLogicalBase`, `LiveRangeMarker`, `SemanticAttr`,
`LayoutResourceInfoAttr`, `SPIRVAsmOperand`) only appear in the
`## Family hierarchy` diagrams and short prose notes; they do not
appear as table rows. Notable opcodes whose semantics cannot fit in a
row of the table have a short `## Notable opcodes` call-out further
down the page.

The `AST origin` column is sourced from
[../../../source/slang/slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp);
when it shows `(synthesized)` the opcode is produced by an IR pass
rather than by lowering, and when it shows `—` no AST mapping was
located in the watched paths.
