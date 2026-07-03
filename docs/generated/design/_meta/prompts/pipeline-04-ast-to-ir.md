# Prompt: pipeline/04-ast-to-ir.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/pipeline/04-ast-to-ir.md` — a description
of how the checked AST becomes the initial Slang IR module.

Audience: a developer modifying lowering for a language feature, or
adding a new IR opcode that the lowering step must produce.

## Required structure

1. `# AST-to-IR Lowering` (title)
2. `## Inputs and outputs` — checked AST → `IRModule`.
3. `## Lowering driver` — describe the entry point in
   [slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp);
   how the lowering visitor walks decls, stmts, exprs, types and emits
   IR via `IRBuilder`.
4. `## IRBuilder and instruction creation` — briefly explain the
   `IRBuilder` API as the canonical way to create instructions; mention
   hoistable / global value deduplication. Link
   [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
   for the opcode catalogue and
   [../../design/ir.md](../../design/ir.md) for the IR design rationale.
5. `## Mapping AST constructs to IR` — short table of the most common
   AST→IR translations:
   - decls → `IRGlobalVar`, `IRFunc`, `IRStructType`, `IRGeneric`
   - stmts → basic blocks, branches, parameters
   - exprs → SSA value instructions
   Cite specific lowering routines by name where useful.
6. `## Generics and existentials` — note that generics are encoded as
   functions in the IR (per [../../design/ir.md](../../design/ir.md)).
   This is an overview; the specialization machinery lives in IR
   passes covered in [05-ir-passes.md](05-ir-passes.md).
7. `## Diagnostics during lowering` — cases where lowering itself can
   emit errors (e.g. unsupported language constructs), linking
   [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).
8. `## Module-level outputs` — what the lowering step produces beyond
   the IR module (entry-point lists, layout intent, etc.).

## Quality checklist (in addition to the universal one)

- [ ] No detailed list of IR opcodes (those belong in
      [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)).
- [ ] No detailed list of IR passes (those belong in
      [05-ir-passes.md](05-ir-passes.md)).
- [ ] Document length under 32 KB.
