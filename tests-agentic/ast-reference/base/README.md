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
recorded under `## Untested claims` because they are
internal to the compiler and unobservable through any test directive
that does not link against the C++ AST headers.


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Every Decl carries a name and a parent ContainerDecl; the same simple name can live in two different parents and be disambiguated via member-access. | functional | [#decl-declbase](../../../docs/llm-generated/ast-reference/base.md#decl-declbase) | [`decl-name-and-parent.slang`](decl-name-and-parent.slang) |
| A DeclRefBase references a declaration possibly with generic substitutions; a single generic function declaration can be referenced at two different substitutions and each reference resolves to the appropriate specialization. | functional | [#declrefbase-val](../../../docs/llm-generated/ast-reference/base.md#declrefbase-val) | [`declref-generic-substitution.slang`](declref-generic-substitution.slang) |
| A DeclRefBase resolves a use of a name to a declaration; when no matching declaration exists in any reachable scope, the checker diagnoses the unresolved reference. | negative | [#declrefbase-val](../../../docs/llm-generated/ast-reference/base.md#declrefbase-val) | [`declref-undeclared-name.slang`](declref-undeclared-name.slang) |
| Every Expr carries a QualType filled in by the checker; that type is what overload resolution dispatches on. | functional | [#expr-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#expr-syntaxnode) | [`expr-carries-type-overload.slang`](expr-carries-type-overload.slang) |
| The checker fills in an Expr's QualType and then verifies it against the surrounding context; a mismatch is diagnosed at the expression's source location. | negative | [#expr-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#expr-syntaxnode) | [`expr-type-mismatch-diagnoses.slang`](expr-type-mismatch-diagnoses.slang) |
| A ModifiableSyntaxNode owns a Modifiers list; the modifiers attached to a Decl actually take effect during checking (a [mutating] member function is permitted to assign through `this`). | functional | [#modifiablesyntaxnode-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifiablesyntaxnode-syntaxnode) | [`modifiable-syntax-node-modifier-effects.slang`](modifiable-syntax-node-modifier-effects.slang) |
| Modifier derives from SyntaxNode and therefore from SyntaxNodeBase, so a Modifier carries its own SourceLoc; a misplaced attribute is diagnosed at the attribute's column, not at the host decl's column. | negative | [#modifier-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifier-syntaxnode) | [`modifier-carries-source-loc.slang`](modifier-carries-source-loc.slang) |
| Modifiers form a linked list attached to a ModifiableSyntaxNode; multiple modifiers stack onto a single declaration and all take effect. | functional | [#modifier-syntaxnode](../../../docs/llm-generated/ast-reference/base.md#modifier-syntaxnode) | [`modifier-list-multiple-on-decl.slang`](modifier-list-multiple-on-decl.slang) |
| Stmt derives from ModifiableSyntaxNode, so a statement can carry one or more modifiers (e.g. an unroll attribute on a for-statement). | functional | [#stmt-modifiablesyntaxnode](../../../docs/llm-generated/ast-reference/base.md#stmt-modifiablesyntaxnode) | [`stmt-carries-modifier.slang`](stmt-carries-modifier.slang) |
| Every SyntaxNodeBase carries a SourceLoc; the diagnostic for a check-time error points at the originating syntax node's column. | negative | [#syntaxnodebase-nodebase](../../../docs/llm-generated/ast-reference/base.md#syntaxnodebase-nodebase) | [`syntaxnodebase-source-loc-in-diagnostic.slang`](syntaxnodebase-source-loc-in-diagnostic.slang) |
| Every type the front end works with is a Type-as-Val; an ill-formed type expression at a declaration site cannot resolve to any Type and is diagnosed. | negative | [#type-val](../../../docs/llm-generated/ast-reference/base.md#type-val) | [`type-malformed-diagnoses.slang`](type-malformed-diagnoses.slang) |
| Vals (including Types) are deduplicated by the ASTBuilder; two textually identical type expressions denote the same Val, so an overload set keyed on the type resolves identically at each use. | functional | [#val-nodebase](../../../docs/llm-generated/ast-reference/base.md#val-nodebase) | [`val-type-dedup-same-overload.slang`](val-type-dedup-same-overload.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| `NodeBase::_astBuilder` back-pointer — private field; not observable. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `Val::m_operands` generic operand list — internal layout. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `NodeBase::astNodeType` discriminator tag — set internally by `ASTBuilder`; no test directive surfaces it. | (unclassified) | [#astbuilder](../../../docs/llm-generated/ast-reference/base.md#astbuilder) | Reason and explanation to be refined by the next regeneration. |
| The FIDDLE-generated `ASTNodeType` enum and its stable-tag guarantee — a build-system invariant, not a `slangc` behavior. | (unclassified) | [#astnodetype](../../../docs/llm-generated/ast-reference/base.md#astnodetype) | Reason and explanation to be refined by the next regeneration. |
| The catalog of support types (`DeclRef<T>`, `Modifiers`, `QualType`, `SubstitutionSet`, `LookupResult`, `LookupResultItem`, `TypeExp`, `WitnessTable`): these are non-node helper types described in `slang-ast-support-types.h`. Their effects bleed through into other observables (a `QualType` is what an `Expr::type` field carries; a `WitnessTable` participates in interface dispatch), but the support types themselves are not surface-visible. The family-page bundles (`ast-reference/values`, `ast-reference/types`, …) are the better home for any tests that do exist; this base bundle deliberately defers. | (unclassified) | [#modifiers](../../../docs/llm-generated/ast-reference/base.md#modifiers) | Reason and explanation to be refined by the next regeneration. |
| `Scope` as a `NodeBase` — runtime name-lookup data structure, not a parsed AST node. | (unclassified) | [#scope](../../../docs/llm-generated/ast-reference/base.md#scope) | Reason and explanation to be refined by the next regeneration. |
| `Type::m_astBuilderForReflection` — reflection-only field; not reachable through `slangc` text output we can FileCheck without GPU-bound runtime APIs. | (unclassified) | [#slangc](../../../docs/llm-generated/ast-reference/base.md#slangc) | Reason and explanation to be refined by the next regeneration. |
| `as<T>()` / `dynamicCast<T>()` dispatch on AST nodes — internal C++ API. | needs-unit-test | (unspecified) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| `SyntaxClass<T>` reflection handle — internal C++ type. | needs-unit-test | (unspecified) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#decl-declbase](../../../docs/llm-generated/ast-reference/base.md#decl-declbase) | undocumented-behavior | The doc lists `Decl::inferredCapabilityRequirements` as a field filled in during checking but does not state a user-facing claim about when a capability mismatch is diagnosed; the doc points inward to a private field rather than outward to an observable behavior. The pipeline pages already cover the capability gate; a one-line cross-link from `#decl-declbase` to `pipeline/03-semantic-check.md` (or to a future capabilities doc) would let an agent anchor a capability-check test here. |  |
| [#declcheckstate](../../../docs/llm-generated/ast-reference/base.md#declcheckstate) | undocumented-behavior | The doc mentions `Decl::checkState` (which checking phases have completed) without stating any externally-observable consequence of `checkState`. The phased-check sequencing is described in `pipeline/03-semantic-check.md`; the cross-reference is implicit. |  |
| (unspecified) | undocumented-behavior | The doc lists `Modifier::keywordName: Name*` but does not name a user-facing consequence of the keyword name beyond appearing in diagnostics. The diagnostic-text dependency is implicit and worth a one-line note. |  |
