# Prompt: pipeline/03-semantic-check.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/pipeline/03-semantic-check.md` — a
description of the semantic-checking stage that turns a raw AST into
a fully resolved, type-checked AST ready for IR lowering.

Audience: a developer working on type checking, name resolution,
overload resolution, or interface conformance.

## Required structure

1. `# Semantic Checking` (title)
2. `## Inputs and outputs` — raw AST from the parser; output is the
   same AST with names resolved, types attached, conformances recorded.
3. `## SemanticsVisitor` — describe the entry point in
   [slang-check.cpp](../../../source/slang/slang-check.cpp) and the
   visitor split across the watched files. List the files and one-line
   responsibilities:
   - `slang-check-decl.cpp` — declaration checking
   - `slang-check-expr.cpp` — expression checking
   - `slang-check-stmt.cpp` — statement checking
   - `slang-check-type.cpp` — type checking
   - `slang-check-overload.cpp` — overload resolution
   - `slang-check-conformance.cpp` — interface conformance
   - `slang-check-conversion.cpp` — implicit conversion
   - `slang-check-inheritance.cpp` — inheritance / extension lookup
   - `slang-check-modifier.cpp` — modifier validation
   - `slang-check-shader.cpp` — shader-specific entry-point checks
4. `## Two-pass interaction with the parser` — how
   `UnparsedStmt` / `UnparsedExpr` are realized: the checker triggers
   the second-stage parse for function bodies. Link
   [02-parse-ast.md](02-parse-ast.md) and
   [../../design/parsing.md](../../design/parsing.md).
5. `## Name lookup and `DeclRef`` — short paragraph; cite
   [slang-ast-decl-ref.cpp](../../../source/slang/slang-ast-decl-ref.cpp)
   and [../../design/decl-refs.md](../../design/decl-refs.md) as
   further reading.
6. `## Generic specialization and constraints` — overview only; the
   deep details live in
   [../../design/interfaces.md](../../design/interfaces.md).
7. `## Synthesizing implicit code` — how
   [slang-ast-synthesis.cpp](../../../source/slang/slang-ast-synthesis.cpp)
   adds default conformance witnesses, generated members, etc.
8. `## Failure modes` — diagnostics and recovery; link
   [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).

## Quality checklist (in addition to the universal one)

- [ ] Each `slang-check-*.cpp` file in the watched paths is mentioned
      at least once.
- [ ] No section reproduces type-system theory that is not specific to
      Slang.
- [ ] Document length under 32 KB.
