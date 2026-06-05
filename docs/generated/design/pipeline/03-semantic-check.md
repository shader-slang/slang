---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: a7f01f5c13a93b4962311b4a8303731a575df2231fa88c54d62f0ee4ce433cb4
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Semantic Checking

This document covers the semantic-checking stage: turning a raw AST
into a fully resolved, type-checked AST that is ready to be lowered
into IR. The intended reader is a developer working on type checking,
name resolution, overload resolution, or interface conformance.

## Inputs and outputs

- **Input**: an AST produced by
  [02-parse-ast.md](02-parse-ast.md), with function bodies still in
  `UnparsedStmt` form.
- **Output**: the same AST, but with names resolved (every
  `DeclRef`-bearing node points at the canonical decl), types attached
  (every `Expr` carries a `Type*`), conformances recorded, modifiers
  validated, default conformance witnesses synthesized, and function
  bodies fully parsed and checked.

The result is the input to AST → IR lowering
([04-ast-to-ir.md](04-ast-to-ir.md)).

## SemanticsVisitor

The checker is implemented as a family of visitor subclasses that
share state through `SemanticsContext`. The base visitor lives in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h) and is
declared as:

```cpp
struct SemanticsVisitor : public SemanticsContext
```

The top-level entry point is `checkTranslationUnit` in
[slang-check.cpp](../../../../source/slang/slang-check.cpp), which the
front-end calls once per `TranslationUnitRequest` after parsing has
collected the decls.

### Files and responsibilities

The `slang-check-*.cpp` family in
[source/slang/](../../../../source/slang) splits the work by concern.
Every file collaborates through `SemanticsContext` /
`SemanticsVisitor` declared in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h):

| File | Concern |
| --- | --- |
| [slang-check.cpp](../../../../source/slang/slang-check.cpp) | Entry point; orchestrates the checking phases |
| [slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp) | `Decl` checking — types, signatures, default values, attributes |
| [slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp) | `Expr` checking — type inference, lvalue-ness, conversions |
| [slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp) | `Stmt` checking — control flow, scope rules, return-type validation |
| [slang-check-type.cpp](../../../../source/slang/slang-check-type.cpp) | Resolves `Type` references that appear in `Expr` form |
| [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp) | Overload resolution; ranks candidates produced by lookup |
| [slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp) | Verifies and synthesizes interface conformances |
| [slang-check-conversion.cpp](../../../../source/slang/slang-check-conversion.cpp) | Implicit-conversion ranking and coercion site checks |
| [slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp) | Inheritance, extension lookup, member visibility |
| [slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp) | Validates modifier combinations and attribute arguments |
| [slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp) | Generic constraint solving (`where`-clauses, witness inference) |
| [slang-check-resolve-val.cpp](../../../../source/slang/slang-check-resolve-val.cpp) | Validates `Val` substitution after generic resolution |
| [slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp) | Entry-point checks: stage-specific signatures, parameter rules |
| [slang-check-out-of-bound-access.cpp](../../../../source/slang/slang-check-out-of-bound-access.cpp) | Static detection of literal out-of-bound array indexing |

## Two-pass interaction with the parser

The parser left function and method bodies as `UnparsedStmt` nodes
(see [02-parse-ast.md](02-parse-ast.md)). When the checker reaches
one, it calls `parseUnparsedStmt`
([slang-parser.h](../../../../source/slang/slang-parser.h)) with a
`SemanticsVisitor*` so that the parser can call back into the
checker to disambiguate `<` tokens at parse time. Once the body is
parsed, the checker continues normally over the resulting `Stmt`
tree.

This interleaving means there is no clean parse / check boundary
inside function bodies: parsing and checking happen together,
on demand. The deeper rationale is in
[../../../design/parsing.md](../../../design/parsing.md).

## Name lookup and `DeclRef`

Name resolution produces `DeclRef`s — a decl plus a substitution that
records how its generic and outer-context parameters have been bound.
The algorithmic rules — scope construction, the lookup algorithm,
shadowing, visibility filtering, and overload resolution — live in
the dedicated [../name-resolution/](../name-resolution) subtree.
Start at [../name-resolution/index.md](../name-resolution/index.md).
For the deeper rationale on decl-refs themselves see
[../../../design/decl-refs.md](../../../design/decl-refs.md).

## Generic specialization and constraints

The checker implements generic-parameter resolution through:

- [slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp)
  — accumulates and solves type / value / witness constraints.
- [slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp)
  — finds (or synthesizes) the witness that a type satisfies an
  interface required by a constraint.
- [slang-check-resolve-val.cpp](../../../../source/slang/slang-check-resolve-val.cpp)
  — validates `Val` substitutions after generic resolution.

The full conceptual model (interfaces, witness tables, existential
types) is in
[../../../design/interfaces.md](../../../design/interfaces.md) and
[../../../design/existential-types.md](../../../design/existential-types.md);
this document only points at the implementation.

## Synthesizing implicit code

Some declarations gain members at check time rather than at parse
time: default conformance witnesses, generated comparison /
construction methods, and several built-in conformances are
synthesized in
[slang-ast-synthesis.cpp](../../../../source/slang/slang-ast-synthesis.cpp).
The checker calls into the synthesis machinery whenever it needs a
member that the user did not write but the language guarantees.

## Modifier validation

Modifier-specific checks live in
[slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp):
which modifiers are allowed on which decls, mutually exclusive
combinations, attribute argument types, and HLSL-vs-Slang dialect
differences. Modifier nodes themselves are defined in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).

## Shader-specific checks

[slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)
validates entry points: the function's stage attribute, parameter
modifiers (`in`, `out`, `inout` and stage-specific intrinsics), return
type compatibility with the stage, and resource binding rules.
Failures here surface as diagnostics that reference the
`shader("...")` attribute or the entry-point signature.

## Failure modes

All semantic-checking errors flow through the `DiagnosticSink`
threaded into `SemanticsContext`. Check-level recovery is generally
"continue with a placeholder type" so that one error does not cascade:
unresolved decls become `ErrorType`-typed, and overload resolution
returns a synthetic `errorExpr` rather than aborting. The diagnostic
infrastructure is described in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).

When the checker completes, every `Decl` in the translation unit is
either fully checked or marked errored, and the AST is ready for IR
lowering (see [04-ast-to-ir.md](04-ast-to-ir.md)).
