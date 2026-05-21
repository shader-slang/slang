---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
watched_paths_digest: 599ef31fb3dc8f15cb8601e47192d5216426cc7cfe8e201e324f7beef84eb695
source_doc: docs/llm-generated/ast-reference/base.md
source_doc_digest: e0ba3f54791ca461a6115045b9af60f5aa9444b387f4f5ba5c42a9669750c1ae
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/base

## Intent

Tests verify the abstract AST root classes described in
[`docs/llm-generated/ast-reference/base.md`](../../../docs/llm-generated/ast-reference/base.md):
`NodeBase`, `SyntaxNodeBase`, `SyntaxNode`, `Modifier`,
`ModifiableSyntaxNode`, `DeclBase`, `Decl`, `Stmt`, `Expr`, `Val`,
`Type`, `DeclRefBase`.

The doc is fundamentally about internal compiler structure that
`slangc` does not expose. Most claims at this layer are not directly
observable. The bundle therefore exercises only those claims whose
**consequences** show up in user-facing behavior — overload
resolution by expression type, source locations in diagnostics,
multiple modifiers stacking on a declaration, qualified name
resolution through a parent decl, generic substitution at a
decl-ref site, type-as-Val deduplication. Each test is small and
single-purpose; the bundle is intentionally compact (12 tests)
because the surface area of slangc-observable AST-root behaviors is
small.

The bulk of the doc's structural claims — `NodeBase`'s
`astNodeType` discriminator, the `ASTBuilder` back-pointer, the
private `Val::m_operands` array, the support-type catalog — are
recorded under `## Out of scope (no-GPU runner)` because they are
internal to the compiler and unobservable through any test directive
that does not link against the C++ AST headers.

## Claims enumerated

| Claim ID | Anchor                                                                                                                       | Claim (one line)                                                                                                                  | Tests                                                |
| -------- | ---------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------- |
| C-01     | [#expr-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#expr-syntaxnode)                                        | Every `Expr` carries a `QualType` filled by the checker; the type drives overload resolution.                                     | [`expr-carries-type-overload.slang`](expr-carries-type-overload.slang)                   |
| C-02     | [#expr-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#expr-syntaxnode)                                        | The checker uses the `Expr`'s type to detect type-mismatched contexts and diagnose them.                                          | [`expr-type-mismatch-diagnoses.slang`](expr-type-mismatch-diagnoses.slang)                 |
| C-03     | [#syntaxnodebase-nodebase](../../../docs/llm-generated/ast-reference/base.md#syntaxnodebase-nodebase)                        | A `SyntaxNodeBase` carries a `SourceLoc`; diagnostics point at the offending syntax node's column.                                | [`syntaxnodebase-source-loc-in-diagnostic.slang`](syntaxnodebase-source-loc-in-diagnostic.slang)      |
| C-04     | [#modifier-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifier-syntaxnode)                                | Modifiers form a linked list; multiple modifiers attach to the same declaration and all take effect.                              | [`modifier-list-multiple-on-decl.slang`](modifier-list-multiple-on-decl.slang)               |
| C-05     | [#modifier-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifier-syntaxnode)                                | A `Modifier` (deriving from `SyntaxNode`/`SyntaxNodeBase`) carries its own `SourceLoc`; the diagnostic points at the attribute.   | [`modifier-carries-source-loc.slang`](modifier-carries-source-loc.slang)                  |
| C-06     | [#modifiablesyntaxnode-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifiablesyntaxnode-syntaxnode)        | A `ModifiableSyntaxNode` actually stores modifiers and they influence checking (a `[mutating]` method may assign through `this`). | [`modifiable-syntax-node-modifier-effects.slang`](modifiable-syntax-node-modifier-effects.slang)      |
| C-07     | [#stmt-modifiablesyntaxnode](../../../docs/llm-generated/ast-reference/base.md#stmt-modifiablesyntaxnode)                    | `Stmt` is a `ModifiableSyntaxNode`; a statement can carry a modifier such as `[unroll]`.                                          | [`stmt-carries-modifier.slang`](stmt-carries-modifier.slang)                        |
| C-08     | [#decl-declbase](../../../docs/llm-generated/ast-reference/base.md#decl-declbase)                                            | Every `Decl` carries a name and a parent; member access disambiguates same-named decls in different parents.                      | [`decl-name-and-parent.slang`](decl-name-and-parent.slang)                         |
| C-09     | [#declrefbase-val](../../../docs/llm-generated/ast-reference/base.md#declrefbase-val)                                        | A `DeclRefBase` resolves a use of a name; an unresolved name is diagnosed at the use site.                                        | [`declref-undeclared-name.slang`](declref-undeclared-name.slang)                      |
| C-10     | [#declrefbase-val](../../../docs/llm-generated/ast-reference/base.md#declrefbase-val)                                        | A `DeclRefBase` may carry generic substitutions; the same generic decl is referenced with different substitutions per call site.  | [`declref-generic-substitution.slang`](declref-generic-substitution.slang)                 |
| C-11     | [#val-nodebase](../../../docs/llm-generated/ast-reference/base.md#val-nodebase)                                              | `Val`s (including `Type`s) are hash-consed; textually identical type expressions denote the same `Val`.                           | [`val-type-dedup-same-overload.slang`](val-type-dedup-same-overload.slang)                 |
| C-12     | [#type-val](../../../docs/llm-generated/ast-reference/base.md#type-val)                                                      | Every type the front end works with is a `Type`-as-`Val`; an ill-formed type expression at a declaration site is diagnosed.       | [`type-malformed-diagnoses.slang`](type-malformed-diagnoses.slang)                     |

## Tests in this bundle

| File                                              | Intent     | Doc anchor                          |
| ------------------------------------------------- | ---------- | ----------------------------------- |
| [`expr-carries-type-overload.slang`](expr-carries-type-overload.slang)                | functional | `#expr-syntaxnode`                  |
| [`expr-type-mismatch-diagnoses.slang`](expr-type-mismatch-diagnoses.slang)              | negative   | `#expr-syntaxnode`                  |
| [`syntaxnodebase-source-loc-in-diagnostic.slang`](syntaxnodebase-source-loc-in-diagnostic.slang)   | negative   | `#syntaxnodebase-nodebase`          |
| [`modifier-list-multiple-on-decl.slang`](modifier-list-multiple-on-decl.slang)            | functional | `#modifier-syntaxnode`              |
| [`modifier-carries-source-loc.slang`](modifier-carries-source-loc.slang)               | negative   | `#modifier-syntaxnode`              |
| [`modifiable-syntax-node-modifier-effects.slang`](modifiable-syntax-node-modifier-effects.slang)   | functional | `#modifiablesyntaxnode-syntaxnode`  |
| [`stmt-carries-modifier.slang`](stmt-carries-modifier.slang)                     | functional | `#stmt-modifiablesyntaxnode`        |
| [`decl-name-and-parent.slang`](decl-name-and-parent.slang)                      | functional | `#decl-declbase`                    |
| [`declref-undeclared-name.slang`](declref-undeclared-name.slang)                   | negative   | `#declrefbase-val`                  |
| [`declref-generic-substitution.slang`](declref-generic-substitution.slang)              | functional | `#declrefbase-val`                  |
| [`val-type-dedup-same-overload.slang`](val-type-dedup-same-overload.slang)              | functional | `#val-nodebase`                     |
| [`type-malformed-diagnoses.slang`](type-malformed-diagnoses.slang)                  | negative   | `#type-val`                         |

## Doc gaps observed

- The doc lists `Decl::inferredCapabilityRequirements` as a field
  filled in during checking but does not state a user-facing claim
  about when a capability mismatch is diagnosed; the doc points
  inward to a private field rather than outward to an observable
  behavior. The pipeline pages already cover the capability gate;
  a one-line cross-link from `#decl-declbase` to
  `pipeline/03-semantic-check.md` (or to a future capabilities doc)
  would let an agent anchor a capability-check test here.
- The doc mentions `Decl::checkState` (which checking phases have
  completed) without stating any externally-observable consequence
  of `checkState`. The phased-check sequencing is described in
  `pipeline/03-semantic-check.md`; the cross-reference is implicit.
- The doc lists `Modifier::keywordName: Name*` but does not name a
  user-facing consequence of the keyword name beyond appearing in
  diagnostics. The diagnostic-text dependency is implicit and
  worth a one-line note.

## Out of scope (no-GPU runner)

(In this bundle the section heading is used for "claims unobservable
through any allowed test directive", not literally GPU-bound claims.
The doc is overwhelmingly about internal compiler structure.)

- `NodeBase::astNodeType` discriminator tag — set internally by
  `ASTBuilder`; no test directive surfaces it.
- `NodeBase::_astBuilder` back-pointer — private field; not
  observable.
- `as<T>()` / `dynamicCast<T>()` dispatch on AST nodes — internal
  C++ API.
- `Val::m_operands` generic operand list — internal layout.
- `Type::m_astBuilderForReflection` — reflection-only field; not
  reachable through `slangc` text output we can FileCheck without
  GPU-bound runtime APIs.
- `SyntaxClass<T>` reflection handle — internal C++ type.
- `Scope` as a `NodeBase` — runtime name-lookup data structure,
  not a parsed AST node.
- The catalog of support types
  (`DeclRef<T>`, `Modifiers`, `QualType`, `SubstitutionSet`,
  `LookupResult`, `LookupResultItem`, `TypeExp`, `WitnessTable`):
  these are non-node helper types described in
  `slang-ast-support-types.h`. Their effects bleed through into
  other observables (a `QualType` is what an `Expr::type` field
  carries; a `WitnessTable` participates in interface dispatch),
  but the support types themselves are not surface-visible. The
  family-page bundles (`ast-reference/values`,
  `ast-reference/types`, …) are the better home for any tests that
  do exist; this base bundle deliberately defers.
- The FIDDLE-generated `ASTNodeType` enum and its stable-tag
  guarantee — a build-system invariant, not a `slangc` behavior.
