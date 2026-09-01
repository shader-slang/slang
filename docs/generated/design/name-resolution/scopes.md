---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T14:05:45Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 2d1b0424ee67205473f3a56c4db750bce6b7847387448b1952bed261cc3cca46
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Scopes

This document describes the `Scope` data structure that drives Slang's
name resolution, the AST node kinds that introduce a new scope, and how
the parser threads scopes through the AST as it builds it. The
intended reader is a developer modifying scope construction, adding a
new scope-bearing AST node, or trying to understand why a given
identifier is in scope at a particular source location.

For the lookup algorithm itself, see [lookup.md](lookup.md). For
visibility filtering, see [visibility.md](visibility.md). For how the
overall pipeline gets here, see
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md).

## Source

Scopes are declared in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h). The
`Decl` subclasses that own scopes are declared in
[slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h), and the
`Stmt` subclasses that own scopes are declared in
[slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h). Scope
construction during parsing happens in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp); the
`addSiblingScopeForContainerDecl` helper used by semantic-checking and
session/module setup code is defined in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp).

## Concepts

- `Scope` (lines 112-128 of
  [slang-ast-base.h](../../../../source/slang/slang-ast-base.h)) — a
  three-field record:
  - `ContainerDecl* containerDecl` (line 121) — the decl whose members
    are the contents of this scope.
  - `Scope* parent` (line 124) — the next scope to consult when a name
    is not found in `containerDecl`.
  - `Scope* nextSibling` (line 127) — the next scope to consult at the
    *same* level before falling through to `parent` (see "Sibling
    scopes" below). The comment in the header notes that `containerDecl`
    is deliberately an unowned pointer so a `Scope` cannot keep an AST
    node alive.
- `ContainerDecl` (abstract, declared in
  [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)) — the
  `Decl` subclass that has child decls. Every `ContainerDecl` carries
  an `ownedScope` field whose `containerDecl` points back to the owning
  decl. This is the canonical way an AST node "owns" a scope.
- `ScopeDecl` (line 590 of
  [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)) — a
  synthetic `ContainerDecl` used to attach a scope to a statement.
  `ScopeDecl` instances do not appear in the surface syntax; they are
  created by the parser for any statement that introduces a local
  scope.
- `ScopeStmt` (abstract, lines 16-21 of
  [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h)) — the
  abstract base of statements that own a scope. It carries a single
  `ScopeDecl* scopeDecl` field; the actual `Scope*` is
  `scopeDecl->ownedScope`.
- `BlockStmt` (line 41 of
  [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h)) — the
  concrete `{ ... }` block; the most common `ScopeStmt`.

## Rules

### Scope-bearing AST nodes

The nodes listed below either own a `Scope` directly (a `ContainerDecl`
via `ownedScope`) or declare a `ScopeStmt::scopeDecl` field. The
"How the scope is attached" column distinguishes a node that *always*
gets a fresh scope from one whose `scopeDecl` field is only populated on
some parser paths — see the notes after the table and the edge-case
section for the statements where the parser does not push a fresh scope.
Citations point at the concrete class in the header.

| Node kind | Header | How the scope is attached |
| --- | --- | --- |
| `ModuleDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 807) | `ContainerDecl::ownedScope` |
| `NamespaceDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 799) | `ContainerDecl::ownedScope` |
| `FileDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 854) | `ContainerDecl::ownedScope` |
| `AggTypeDecl` and its subclasses `StructDecl`, `ClassDecl`, `InterfaceDecl`, `EnumDecl`, `SynthesizedStructDecl`, `GLSLInterfaceBlockDecl`, `AssocTypeDecl`, `GlobalGenericParamDecl`, `ThisTypeDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (common base `AggTypeDeclBase` at line 360, plus `SynthesizedStructDecl` line 420, `GLSLInterfaceBlockDecl` line 434, `ThisTypeDecl` line 478, `AssocTypeDecl` line 567, `GlobalGenericParamDecl` line 575) | `ContainerDecl::ownedScope`; `ThisTypeDecl` is the synthesized member of an `InterfaceDecl` representing the abstract `This` type, reached through `InterfaceDecl::getThisTypeDecl` (line 488), and it is never pushed as a parser scope, so it inherits the `ownedScope` field but the field stays null |
| `ExtensionDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 367) | `ContainerDecl::ownedScope` |
| `GenericDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 929) | `ContainerDecl::ownedScope`; the scope contains the generic parameters |
| `CallableDecl` and its subclasses `FuncDecl`, `ConstructorDecl`, `SubscriptDecl`, `AccessorDecl`, `FuncAliasDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 612 onward; `FuncAliasDecl` line 653) | `ContainerDecl::ownedScope`; the scope contains the parameter decls |
| `PropertyDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 698) | `ContainerDecl::ownedScope` |
| `SemanticDecl`, `AttributeDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (`SemanticDecl` line 732, `AttributeDecl` line 1171; both direct `ContainerDecl` subclasses) | `ContainerDecl::ownedScope` |
| `ScopeDecl` | [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) (line 590) | `ContainerDecl::ownedScope`; attached to a `ScopeStmt` |
| `BlockStmt` | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (line 41) | `ScopeStmt::scopeDecl`; the parser always pushes a fresh `ScopeDecl` in `parseBlockStatement` ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 7130) |
| `ForStmt`, `UnscopedForStmt` | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (lines 216-231) | `ScopeStmt::scopeDecl`; `Parser::ParseForStatement` ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 7392) assigns `scopeDecl` but only pushes it for the scoped `ForStmt` — `UnscopedForStmt` reuses the parent scope for HLSL compatibility |
| `WhileStmt`, `DoWhileStmt` | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (lines 234-247) | declare `ScopeStmt::scopeDecl` (via `LoopStmt` -> `BreakableStmt` -> `ScopeStmt`), but the parser (`ParseWhileStatement`, `ParseDoWhileStatement`) does **not** create or assign a fresh `ScopeDecl`; the loop body owns its own scope only when it is a `BlockStmt` |
| `CompileTimeForStmt` | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (line 251) | `ScopeStmt::scopeDecl` |
| `GpuForeachStmt` | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (line 198) | `ScopeStmt::scopeDecl` |
| `SwitchStmt`, `TargetSwitchStmt`, `StageSwitchStmt` (`BreakableStmt` subclasses) | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (lines 116-154) | declare `ScopeStmt::scopeDecl`, but the parser does not assign it: `ParseSwitchStmt` ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 6572) gives the body a scoped `BlockStmt`, and `parseTargetSwitchStmtImpl` (line 6603) creates a per-case `ScopeDecl` rather than one on the statement |
| `CatchStmt` (catch handler) | [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h) (line 306); a fresh `ScopeDecl` is pushed in `Parser::ParseDoCatchStatement` ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 7482) | indirect, through the surrounding `ScopeDecl` the parser creates |
| `parseIfLetStatement` (synthetic) | [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) (line 7284) | a fresh `ScopeDecl` is pushed for the unwrapped variable |
| `LambdaExpr` (parameter scope) | the parser creates and pushes `lambdaExpr->paramScopeDecl` in `parseLambdaExpr` ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines 8426-8427); `LambdaDecl` itself ([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line 682) is a `StructDecl` and owns a scope as an aggregate | dedicated `ScopeDecl` for the lambda parameter list, owned by the expression, not by `LambdaDecl` |

Several AST nodes do *not* own a fresh scope even though syntactically
they look like they might:

- `IfStmt` ([slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h)
  line 83) does not own a scope; its branch bodies parse as
  `BlockStmt`s that own one. `if (let x = ...)`
  is the exception: `Parser::parseIfLetStatement`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  7284) synthesizes additional `ScopeDecl`s for the unwrapped
  variable.
- `SeqStmt`, `DeclStmt`, and other `Stmt` subclasses that are not
  `ScopeStmt` simply live inside the enclosing scope.

A declaration used directly as the body of a statement that owns no
scope — `while (c) int x = 1;`, and the same shape under `do` or
`if` — is accepted rather than rejected, and the name leaks:
`Parser::parseVarDeclrStatement`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
7255) hands the declaration to `currentScope->containerDecl` (line
7260), which is the *enclosing* container when no scope was pushed for
the statement, so `x` stays visible after the loop. Only a `{ ... }`
body isolates it, because only the `BlockStmt` pushes a `ScopeDecl`.

The two switch forms differ in the same way. A plain `switch` gives its
whole body one scoped `BlockStmt` (`ParseSwitchStmt`,
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
6588), so a local declared under one `case` label is still in scope in
textually later cases of the same `switch`, and out of scope after it.
`__target_switch` and `__stage_switch` instead push and pop a
`ScopeDecl` around *each* case group (`parseTargetSwitchStmtImpl`,
lines 6622-6623 and 6702), so a local declared in one case is not
visible from any other case of the same statement.

The scope holding a lambda's parameter list ends with the expression
that introduced it: `parseLambdaExpr`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
8441) pushes `paramScopeDecl` before the parameter list (line 8446) and
pops it after the body (line 8467), so a parameter is visible
throughout the body — block form or single-expression form — and
undefined once the expression ends.

```slang
// `applyF` is an `IFunc`-constrained helper; `c` is a local of the
// enclosing function.
int r = applyF((int p) => p * c, 5);
int q = p; // error: undefined identifier 'p'
```

### Parser scope construction

The parser carries the current scope pointer as a member field
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
121-123 at `source_commit`):

- `Parser::currentScope` (line 123) — scope where new decl definitions
  are inserted.
- `Parser::currentLookupScope` (line 122) — scope where in-parser
  expression lookup starts (kept in sync with `currentScope` via
  `resetLookupScope`, line 142).
- `Parser::outerScope` (line 121) — the initial scope at the start of
  parsing.

Three helper methods push and pop scopes
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
144-170):

- `PushScope(ContainerDecl*)` (line 144) — allocates a new `Scope`,
  links its `parent` to `currentScope`, writes itself back into
  `containerDecl->ownedScope`, and updates `currentScope`.
- `pushScopeAndSetParent(ContainerDecl*)` (line 160) — same plus
  assigning `containerDecl->parentDecl = currentScope->containerDecl`
  before pushing. This is the helper most parsing code calls.
- `PopScope()` (line 166) — restores `currentScope = currentScope->parent`.

All three end by calling `resetLookupScope()`, so `currentLookupScope`
never drifts from `currentScope` across a push or pop.

A representative chain that arises in a Slang file is shown below.
Each box is the `ContainerDecl` referenced by a `Scope::containerDecl`,
and arrows point from a child scope to its `parent`.

```mermaid
flowchart BT
  block["ScopeDecl (BlockStmt body)"]
  func["FuncDecl"]
  generic["GenericDecl (T)"]
  structDecl["StructDecl"]
  ns["NamespaceDecl Foo"]
  moduleDecl["ModuleDecl"]
  block --> func
  func --> generic
  generic --> structDecl
  structDecl --> ns
  ns --> moduleDecl
```

The same parser-call chain that produces this looks roughly like:
`parseNamespaceDecl` -> `Parser::ParseStruct` (line 6362) ->
`parseDeclBody` (line 6284) -> `parseOptGenericDecl` ->
`parseFuncDecl` (line 5088) -> `parseBlockStatement`. The declaration
parsers in that chain call `PushScope` (`parseDeclBody` line 6286,
`parseOptGenericDecl` line 1794, `parseFuncDecl` lines 5099 and 5112),
which creates the `Scope`, parents it to `currentScope`, and stores it
in `ownedScope`; those declarations get their `parentDecl` from the
surrounding construction path. A `ScopeDecl` introduced by a statement,
such as the block in `parseBlockStatement` (line 7142), instead calls
`pushScopeAndSetParent`, which sets `parentDecl` from the current
scope's container before pushing (line 160). Every path matches its
push with a `PopScope` on the way out.

### Sibling scopes

`Scope::nextSibling` lets one scope chain consult several containers
at the same nesting level. The constructor is the free function
`addSiblingScopeForContainerDecl` defined in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
(lines 324-331); it allocates a fresh `Scope`, points it at the
secondary `ContainerDecl`, and splices it into the existing
`nextSibling` list of the destination scope. A convenience overload
that takes a destination `ContainerDecl*` (lines 316-322) simply
forwards to `dest->ownedScope`.

Four concrete uses of sibling scopes are visible in the source:

1. **`FileDecl` per source file in a multi-file module.** A module
   that is split across multiple `__include`d files has one
   `ModuleDecl` plus one `FileDecl` per source file. Each `FileDecl`
   is attached to the module's scope as a sibling so that lookup
   inside the module sees the union of all files'
   members — see
   [slang-session.cpp](../../../../source/slang/slang-session.cpp)
   line 2295 and `SemanticsVisitor::importFileDeclIntoScope` in
   [slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
   line 17023.
2. **Imported modules.** When module B imports module A, the
   checker adds A's scope as a sibling of B's scope so that names
   from A are reachable in B without explicit qualification — see
   `SemanticsVisitor::importModuleIntoScope` in
   [slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
   line 17032. Not every scope already on module A's sibling chain gets
   re-exported into B, though. The loop is filtered by
   `isOwnModuleOrIncludedFileScope` (called at line 17066, defined at
   [slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
   line 333), which admits only A's own scope and `FileDecl`s whose
   `parentDecl` is A itself. Without that filter, a `using namespace
   Foo;` written in A's primary file, or a module C that A merely
   `import`ed, would arrive on A's `nextSibling` chain and then leak
   into B — making a plain `import` silently transitive. See
   [visibility.md](visibility.md) for the reachability rules this
   upholds.
3. **Multiple `namespace Foo {}` declarations of the same logical
   namespace.** When the same namespace name reappears, the parser
   reuses the existing `NamespaceDecl`
   (`parseNamespaceDecl`,
   [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
   4431) so that further declarations are inserted into the same
   container. The semantic checker links siblings in
   `SemanticsDeclScopeWiringVisitor::visitNamespaceDecl`
   ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
   line 17389, calling `addSiblingScopeForContainerDecl` at line 17416)
   when more than one `NamespaceDecl` exists.
4. **`using` declarations.** `SemanticsDeclScopeWiringVisitor::visitUsingDecl`
   ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
   line 17325) checks the `using` argument and, for each
   namespace-like (`NamespaceDeclBase`) target it names, calls
   `addSiblingScopeForContainerDecl` (line 17355) to splice that
   namespace's owned/sibling scopes into the `using` decl's scope, so
   the namespace's members become reachable without qualification.

A three-module shape shows what the filter in use case 2 keeps and what
it drops. Module `C` declares `namespace Foo`; module `A` is

```slang
import C;            // plain import: A does not re-export C
using namespace Foo; // splices Foo onto A's module scope sibling chain
```

and module `B` is `import A;`. `B` sees `A`'s own members and the
members of every file `A` pulls in with `__include`, because those are
the only two shapes `isOwnModuleOrIncludedFileScope` admits. It does
not see `Foo`'s members unqualified — the `using`-spliced
`NamespaceDecl` scope is neither `A` nor a `FileDecl` parented to `A` —
and it does not see `C`, whose scope and whose `FileDecl`s all carry a
`parentDecl` of `C`. Writing `__exported import C;` in `A` is what
makes `C` reachable from `B`: `importModuleIntoScope` recurses into
nested `import`s that carry the `ExportedModifier`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 17085-17092).

When two scopes on the same sibling chain supply the same name, neither
wins by position. `_lookUpInScopes`
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) line
806) walks every `nextSibling` link before it considers stopping, so a
hit in one sibling does not preclude a hit in another and both
declarations land in one `LookupResult`. Overloadable declarations then
form an overload set that overload resolution ranks; anything else
remains an `OverloadedExpr` whose use site is reported as
`Diagnostics::AmbiguousReference` by
`SemanticsVisitor::diagnoseAmbiguousReference`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
line 1497). See [lookup.md](lookup.md) for the refinement steps that
run before that point.

### Implicit scopes

A few intermediate scopes have no direct surface-syntax representation
but are still created at parse time:

- **Generic parameter list.** `parseOptGenericDecl`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  1787) creates a `GenericDecl` and pushes its scope *before* parsing
  the inner decl. The generic parameters live in the `GenericDecl`'s
  scope; the inner decl's own scope is its child.
- **Extension body.** `ExtensionDecl` owns its own scope, but the
  members of the type it extends are *not* in the extension's scope
  chain; they are reached through member lookup at check time.
- **Interface requirement list.** `InterfaceDecl` owns a single scope
  for its requirements; the default-impl bodies parse against a
  derived `InterfaceDefaultImplDecl` ([slang-ast-decl.h line
  944](../../../../source/slang/slang-ast-decl.h)) that is itself a
  `GenericDecl` subclass and thus has its own scope. An associated
  type's constraint clause does *not* go into the
  `AssocTypeDecl`'s own scope: when `parseAssocType`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4293) sees that the current scope's `containerDecl` is an
  `InterfaceDecl`, it sets `constraintTarget` to the *enclosing
  interface* so each `GenericTypeConstraintDecl`
  ([slang-ast-decl.h line 979](../../../../source/slang/slang-ast-decl.h))
  produced by `associatedtype A : IBar` or `associatedtype A where A : IBar`
  becomes a sibling member of the associated type. The dedicated
  `__constraint` keyword (`parseInterfaceConstraintDecl`,
  [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4335) inserts a `GenericTypeConstraintDecl` directly into the
  interface scope the same way. All three surface forms therefore land
  in one scope as parallel requirement members.
- **`if (let x = ...)` desugaring.** `parseIfLetStatement`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  7284) creates `ScopeDecl`s for the temporary `$OptVar` binding and
  for the user-visible unwrapped variable inside the positive branch.

### Scope walking order during lookup

Lookup walks the chain in a fixed order, defined by the lookup entry
points (see [lookup.md](lookup.md)):

1. Visit `currentScope` itself: its `containerDecl`'s direct members.
2. Walk `currentScope->nextSibling` until null, repeating step 1 for
   each sibling.
3. Move to `currentScope->parent` and repeat from step 1.
4. Stop when the parent chain reaches `nullptr`.

The same `parent`-then-`nextSibling` traversal is reused outside the
main lookup path by `findClosestInScopeName` in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
(line 5202): when a `VarExpr` fails to resolve, the checker walks the
scope chain (skipping the core module so its thousands of builtins do
not produce spurious matches) looking for a sufficiently close
edit-distance spelling and attaches a "did you mean" suggestion to the
`Diagnostics::UndefinedIdentifier` diagnostic (line 5361).

The detailed lookup algorithm — masks, inheritance walks,
transparent-member injection, deduplication — lives in
[lookup.md](lookup.md). This page only states the order in which scopes
are consulted.

## Edge cases and failure modes

- **Empty block scope.** A `BlockStmt` whose body contains zero
  declarations still has a fresh `ScopeDecl`: the parser pushes one
  unconditionally and gives the empty block an `EmptyStmt`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
  7217-7223), so the scope exists whether or not anything is declared
  in it. The per-decl `Decl::hiddenFromLookup` flag
  ([slang-ast-base.h](../../../../source/slang/slang-ast-base.h) line
  803) is a separate mechanism that only engages for blocks that do
  contain declaration statements: `SemanticsStmtVisitor::visitBlockStmt`
  ([slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp)
  line 82) sets it on each `DeclStmt` of the body's `SeqStmt` (lines
  106-116). The flag is cleared as the checker walks past each
  `DeclStmt` (line 73); the lookup-side check is in
  [slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) line 179.
- **`UnscopedForStmt`.** When the source language is HLSL,
  `Parser::ParseForStatement`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  7392) creates an `UnscopedForStmt` and *skips* the
  `pushScopeAndSetParent` call, so the `for` loop's initialization
  variable leaks into the surrounding scope as HLSL semantics demand.
- **Multiple `namespace Foo {}` siblings.** `parseNamespaceDecl`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4431) reuses the first `NamespaceDecl` it finds in the parent, so
  all subsequent declarations parse into the same `ContainerDecl`.
  Lookup still has to walk sibling-linked `NamespaceDecl`s across
  modules; that is what `addSiblingScopeForContainerDecl` is for.
- **`GenericDecl` parameter scope vs inner-decl scope.** A reference
  to a generic type parameter `T` inside the inner decl resolves
  through the inner scope's `parent`, which is the `GenericDecl`'s
  scope. A sibling of the outer decl that mentions `T` cannot reach
  it — its scope chain does not pass through the `GenericDecl`.
- **`__constraint` subject must not be `This`.** A
  `GenericTypeConstraintDecl` is only allowed as a child of an
  `InterfaceDecl` (or of a `GenericDecl`); `isDeclAllowed`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp))
  enforces the placement. When the relocated decl lands
  in the interface scope, the header visitor
  `SemanticsDeclHeaderVisitor::visitGenericTypeConstraintDecl`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  line 4394) further rejects a `__constraint` whose subject resolves to
  the bare `This` type — that is the role of the inheritance clause —
  diagnosing `Diagnostics::ConstraintSubjectCannotBeThisType`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  line 4412) and replacing the subject with the error type. Constraints
  on associated types (e.g. `This.A : IBar`) are permitted.
- **`ExtensionDecl` members are not in the extension's scope chain.**
  Lookup *into* a type that has an active extension must walk the
  extension's members explicitly; the extension scope is not
  configured as a sibling of the extended type's scope. The relevant
  helper is in [slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp)
  and is documented in [lookup.md](lookup.md).
- **`UsingDecl`.** A `using` declaration ([slang-ast-decl.h line
  861](../../../../source/slang/slang-ast-decl.h)) captures
  `parser->currentScope` at parse time (see `parseUsingDecl` in
  [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4543). The injection into the surrounding scope happens at check
  time, not at parse time:
  `SemanticsDeclScopeWiringVisitor::visitUsingDecl`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  line 17325) adds each named namespace/module as a sibling scope via
  `addSiblingScopeForContainerDecl`. If the argument does not resolve
  to any namespace-like entity, no sibling is added and the checker
  diagnoses `Diagnostics::ExpectedANamespace`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  line 17384). The destination is always the scope captured at parse
  time — `parseUsingDecl` stores `parser->currentScope` in `decl->scope`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  4552) and checking passes that same pointer unchanged — so what
  checking changes is that scope's `nextSibling` chain, not the scope
  the names land in.
- **`UnparsedStmt` and deferred body parsing.** The parser runs in one
  of two stages, `ParsingStage::Decl` or `ParsingStage::Body`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
  86-98). In the `Decl` stage a function body is not parsed at all; it
  is recorded as an `UnparsedStmt` (created at line 2246) that captures
  both `currentScope` and `outerScope` ([slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h)
  lines 53-61). `parseUnparsedStmt` (line 9951) later restores exactly
  those two pointers and re-enters with `stage = ParsingStage::Body`, so
  a deferred body resolves names against the scope chain that was live
  at its declaration site rather than wherever the deferred parse
  happens to be driven from. This is why the captured scopes must be
  stored on the statement instead of being recomputed.
- **Synthesized constraint decls in an interface scope.** Besides the
  three parsed forms above, `FuncConstraintDecl`
  ([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line
  1037) — a `GenericTypeConstraintDecl` subclass — also lands in an
  interface or generic scope as a requirement member, but it is
  synthesized during header checking, so it has no parser path and no
  scope of its own.
- **Empty parser scope.** Pushing a `Scope` whose `containerDecl`
  is null is not supported. `Parser::PushScope` requires the
  `ContainerDecl*` overload to allocate one; the bare-`Scope*`
  overload (line 154) exists only for restoring a pre-built scope.

## See also

- [lookup.md](lookup.md) — the lookup algorithm that walks the
  scope chain.
- [visibility.md](visibility.md) — the visibility filter that runs
  on top of lookup.
- [overload-resolution.md](overload-resolution.md) — overload
  ranking that consumes filtered lookup results.
- [../ast-reference/base.md](../ast-reference/base.md) — the
  reference for `NodeBase`, `Decl`, `Scope`, and other base types.
- [../ast-reference/declarations.md](../ast-reference/declarations.md)
  — per-class reference for every `Decl` subclass.
- [../ast-reference/statements.md](../ast-reference/statements.md)
  — per-class reference for every `Stmt` subclass.
- [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md) — the
  parsing-stage overview that drives scope construction.
- [../glossary.md](../glossary.md) — glossary entries for
  `scope`, `decl-ref`, `lookup result`, `name resolution`.
