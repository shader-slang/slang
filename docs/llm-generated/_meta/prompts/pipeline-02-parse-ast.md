# Prompt: pipeline/02-parse-ast.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/pipeline/02-parse-ast.md` — a detailed
description of how the token stream becomes the AST.

Audience: a developer adding new syntax, modifying an AST node, or
debugging a parse error.

## Required structure

1. `# Parse and AST Construction` (title)
2. `## Inputs and outputs` — token array in, AST root out.
3. `## Parser`:
   - File pointer
     ([slang-parser.cpp](../../../source/slang/slang-parser.cpp)).
   - Recursive-descent design, with a note that arbitrary lookahead is
     possible because tokens are pre-collected.
   - The two-stage parsing strategy (decl scan, then deferred body
     parsing) — but keep this short and link the formal explanation to
     [../../design/parsing.md](../../design/parsing.md) for the
     historical narrative if useful.
   - "Syntax-as-declaration" model: how keywords are resolved by lookup
     in the active environment, not by hard-coded keyword tables.
     Reference [../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).
4. `## AST data model`:
   - The hierarchy rooted at the classes declared in the
     `slang-ast-*.h` watched paths. Identify the top-level base
     (`NodeBase` or equivalent — verify in
     [slang-ast-base.h](../../../source/slang/slang-ast-base.h)) and
     the major child families (Decl, Expr, Stmt, Type, Modifier, Val).
   - The role of `ASTBuilder` (allocation, hash-consing of types).
   - Note that AST nodes use C++ RTTI / `as<>()` casting and that node
     classes are partly generated through the `FIDDLE` macro system —
     cite the build artefacts in `build/source/slang/fiddle/` only as
     a forward reference; do not assume they are present.
5. `## Generics ambiguity` — short paragraph on the `<` heuristic and
   the disambiguation strategy. Link to the deeper treatment in
   [../../design/parsing.md](../../design/parsing.md).
6. `## Modifier parsing` — how modifiers attach to declarations
   ([slang-ast-modifier.h](../../../source/slang/slang-ast-modifier.h)).
7. `## Failure modes` — error-recovery strategy and how parse errors
   become diagnostics
   (link [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).

## Quality checklist (in addition to the universal one)

- [ ] Do not enumerate every AST node class. Cite representative
      examples and link to the headers.
- [ ] Do not duplicate grammar productions; defer to
      [../syntax-reference/grammar.md](../syntax-reference/grammar.md).
- [ ] Document length under 32 KB.
