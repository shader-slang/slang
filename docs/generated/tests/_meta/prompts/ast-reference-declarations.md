# Prompt: docs/generated/tests/ast-reference/declarations/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/ast-reference/declarations/`,
anchored to
[`docs/generated/design/ast-reference/declarations.md`](../../../design/ast-reference/declarations.md).

Audience: nightly CI. The bundle exercises the **concrete `Decl` and
`DeclBase` subclasses** in the Slang AST through their **user-observable
consequences** at parse / type-check / emit time. The doc enumerates a
large family of declaration node classes (`FuncDecl`, `VarDecl`,
`StructDecl`, `ClassDecl`, `InterfaceDecl`, `EnumDecl`, `ExtensionDecl`,
`GenericDecl`, `TypeAliasDecl`, `NamespaceDecl`, `PropertyDecl`,
`SubscriptDecl`, `ConstructorDecl`, accessor decls, import/using/empty
decls, ...). Each concrete declaration kind has a documented role; a
test in this bundle picks one declaration kind, writes the smallest
piece of Slang code that exercises that role, and verifies the
documented behavior is observable.

This bundle is **large** by manifest design (size cap 80). Aim for
25–50 tests so the major declaration kinds get coverage. Skip kinds
whose only observable surface is internal AST shape (see
`## Untested claims` below).

## The translation rule: claims to observations

`declarations.md` describes the **shape** of internal AST decl classes
(field names, parent class, the parser entry point that produces the
class). Slang as a compiler does not expose its AST. So a claim such
as "`LetDecl` derives from `VarDecl` and is the immutable variant" is
testable only through its observable consequence: a `let`-bound name
cannot be reassigned and the compiler diagnoses an attempt to do so.
This bundle therefore follows the rule:

- **Testable** ⇔ "if the documented role of this declaration kind
  were not implemented, the program-text behavior we wrote would
  change in a way `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the parser allocated, the value of a private field, the
  exact spelling of a key field, or the in-memory layout of a
  member list."

Concretely, here is the split for this doc:

### Observable claims (write tests for these)

For each concrete decl kind in `## Nodes`, the **role** column (the
prose "Summary" cell) is the testable claim. Examples:

- `VarDecl` is an "ordinary mutable variable" — declare one, mutate
  it, observe the mutation.
- `LetDecl` is the "`let` variable; immutable" — declare one, try to
  assign through it, observe the diagnostic.
- `StructDecl` is a "user-defined struct" (value type) — declare,
  construct, read a field.
- `ClassDecl` is a "user-defined class (reference type)" — multiple
  references to the same instance observe the same mutation.
- `EnumDecl` + `EnumCaseDecl` — declare an enum, switch over its
  cases, observe each branch.
- `InterfaceDecl` + `InheritanceDecl` — declare an interface, conform
  a struct to it, dispatch through an interface-typed parameter.
- `ExtensionDecl` — write `extension T { ... }`; call the added
  member from outside the extension.
- `TypeAliasDecl` / `TypeDefDecl` — alias a type; use the alias
  where the original type is expected.
- `NamespaceDecl` — declare a namespace; access a member with
  `Ns::name`. The doc explicitly states that **two textual
  `namespace N { }` blocks with the same name collapse into one
  `NamespaceDecl`**; verify by spreading members across two
  same-named blocks.
- `FuncDecl` — declare a function with parameters; call it.
- `ConstructorDecl` — declare `__init(...)`; construct with the
  initializer.
- `SubscriptDecl` — declare `__subscript`; index with `[]`.
- `PropertyDecl` + `GetterDecl` / `SetterDecl` — declare a property
  with explicit `get` / `set`; observe each accessor firing.
- `GenericDecl` + `GenericTypeParamDecl` — declare a generic;
  observe specialization at two different type arguments.
- `GenericTypeConstraintDecl` — a `where T : I` constraint;
  positive: call satisfies the constraint, negative: call from a
  context that does not satisfy diagnoses.
- `AssocTypeDecl` — interface with `associatedtype T`; concrete
  implementation provides `T`.
- `ImportDecl` / `UsingDecl` — `import M;` brings a module in,
  `using` brings a name into scope.
- `EmptyDecl` — empty declaration carrying modifiers (the doc cites
  GLSL `layout(...) in;` style usage, but a simpler `[modifier];`
  form at module scope works).
- `RequireCapabilityDecl` — `require_capability(...)` at module
  scope.
- `DeclGroup` — `int a, b;` parses as a group, both names visible.
- `LambdaDecl` — a lambda expression produces a closure-struct that
  is callable.
- `FuncAliasDecl` — function alias re-export with `=`.

### Negative claims (one diagnostic test each, where the kind has one)

- `LetDecl` immutability — assignment through a `let`-bound name
  diagnoses.
- `EnumDecl` exhaustiveness — `switch` without `default` over an
  enum diagnoses (or, alternatively, that an undeclared case is
  rejected).
- `InterfaceDecl` requirement — conforming type without an
  interface-required method diagnoses.
- `GenericTypeConstraintDecl` — generic specialization with a type
  that does not satisfy the constraint diagnoses.
- `ConstructorDecl` — calling `__init` with the wrong arity / type
  diagnoses.
- `UsingDecl` / `ImportDecl` — referring to an unimported name
  diagnoses (the negative of the import claim).

### Not testable through slangc (do NOT write tests for these)

The doc carries many claims about the **internal AST shape** of these
declarations — which C++ field stores the type expression, which
class is the parent in the C++ hierarchy, which parser callback
allocates the node, whether `inner` of `GenericDecl` holds the
genericized decl. These are unobservable through `slangc` text I/O.
Record them under `## Untested claims` in `README.md`. Examples:

- That `VarDecl::type` is a `TypeExp` (vs. a resolved `Type*`).
- That `LetDecl` derives from `VarDecl` in C++ (only the immutability
  is observable).
- That `GenericDecl` stores the genericized decl in `inner` (only the
  parameterized behavior is observable).
- That `ExtensionDecl::targetType` is a `TypeExp` (only the eventual
  member lookup is observable).
- That `InheritanceDecl` stores a `RefPtr<WitnessTable>` (only
  successful interface dispatch is observable).
- That `ConstructorDecl::m_flavor` distinguishes UserDefined /
  SynthesizedDefault / SynthesizedMemberInit (only the resulting
  constructor call is observable).
- That `ModuleDecl`, `FileDecl`, `NamespaceDecl` form a three-layer
  nesting in the AST (only the user-visible lookup behavior is
  observable).
- That `EnumCaseDecl` carries `tagVal: IntVal*` (only the resulting
  case-value comparison is observable).
- That `SyntaxDecl` binds a keyword to a parser callback — covered
  by `syntax-reference/keywords-and-builtins/` from the keyword
  side; the AST shape itself is not user-observable.
- That `AttributeDecl::syntaxClass` reflects an attribute class — an
  attribute's effect is observable, but the C++ class identity is
  not.
- The contents of the `Family hierarchy` mermaid graph as a graph —
  abstract intermediate classes (`ContainerDecl`, `CallableDecl`,
  `AggTypeDeclBase`, `FunctionDeclBase`, `VarDeclBase`, ...) carry
  no FIDDLE concrete tag and produce no test by themselves.
- `SynthesizedStructDecl` / `SynthesizedFuncDecl` — created by the
  checker, not by user code; there is no source spelling.
- `UnresolvedDecl` — only appears during deserialization.
- `ThisTypeDecl` / `ThisTypeConstraintDecl` — synthetic helpers
  inside `InterfaceDecl`; their existence is implicit in the
  interface-conformance tests we do write.

If you find yourself thinking "this would verify that the AST node
allocated is class X" or "this would assert that field F holds
sub-class S", stop — that is a source-targeting probe in disguise.
Re-frame as "the documented role of this declaration kind".

## Avoid duplication with sibling bundles

- `syntax-reference/keywords-and-builtins/` — already covers
  **keyword spelling and recognition** (`struct`, `class`, `enum`,
  `__init`, `__subscript`, `let`, `var`, `typealias`, `namespace`,
  `import`, ...). This bundle's tests must go further than "the
  keyword parses": exercise the **declaration's documented role**
  (a member declared in an `extension` is callable from outside the
  extension; an `__init` constructs the struct; a `let` cannot be
  reassigned).
- `pipeline/02-parse-ast/` — already covers **parse-stage decl
  shape** (struct with fields parses; modifier attached to decl;
  function body deferred to check-stage). This bundle's tests
  observe behaviors that show up **after** parsing — type checking,
  overload resolution, interface dispatch, emitted code.
- `ast-reference/base/` — covers the AST root classes
  (`Decl::name`, `Decl::parent`, `DeclRefBase`, etc.). This bundle
  is about the concrete subclass roles, not the abstract bases.

If a claim is best tested in a sibling bundle, do not duplicate it
here. If two bundles seem to want the same test, prefer the more
specific one and cite the doc anchor for the angle this bundle
takes.

## Allowed secondary doc citations

- `docs/generated/design/ast-reference/base.md`
- `docs/generated/design/syntax-reference/grammar.md`
- `docs/generated/design/syntax-reference/keywords-and-builtins.md`
- `docs/generated/design/pipeline/02-parse-ast.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

- `source/slang/slang-ast-decl.h`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-parser.cpp`

You may look at these files to verify that a claim in the doc is
realizable (e.g. that an `EmptyDecl` is reachable from a particular
parse path, that a `RequireCapabilityDecl` is spelled
`require_capability`). You may **not** mine them for behavioral
claims that the doc does not make.

## Required structure

1. `README.md` with the structure named in `_common.md`. List
   internal-AST-shape claims under `## Untested claims` (using the same
   convention as `ast-reference/base/`).
2. 25 to 50 `.slang` test files (size cap 80). The bundle is large by
   manifest design because the doc has many concrete decl kinds.

## Test directives

Most declaration claims are **target-independent**: they parse and
check before any backend runs, and a CPU-bound observation
(`printf`-ing an interpreted result, or producing a diagnostic) is
enough to verify them.

- `//TEST:INTERPRET(filecheck=CHECK):` — **primary directive**. Use
  for any decl-kind claim whose effect is observable by `printf`-ing
  a value the compiler computed (a struct field read, a method call
  result, an interface-dispatched return value, an enum-case branch
  selection).
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — use for the documented
  "this misuse is rejected" claims (let immutability, interface
  conformance missing a required member, generic constraint not
  satisfied, undeclared name).
- `//TEST:SIMPLE(filecheck=CHECK):-target <X> -entry main -stage <S>` —
  use **only** when the declaration affects emit text observably and
  the per-target spelling matters. Most decl claims do not need
  this. Justified examples:
  - struct field ordering / layout in emitted text;
  - a modifier-bearing decl (`groupshared`, `cbuffer`) whose
    spelling differs per backend (defer to
    `syntax-reference/keywords-and-builtins/` if that bundle already
    covers it).

Per `_common.md`'s multi-backend rule, do NOT add multi-target
directives for pre-backend claims; that is repetition of identical
behavior.

## Cast and observation reminders (carried from `_common.md` / pilot)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi`.
- `slangi` `printf` does not support `%s`. For string observation,
  use `//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute`
  and FileCheck the emitted C++ source for the string literal.
- `static const int x = N;` at file scope is the cleanest pattern
  for asserting a compile-time-known result.
- The runner's "Suggested annotations" output is the source of truth
  for diagnostic caret positions. Hand-counting carets is unreliable.
- Caret `^` placement in `//CHECK:` requires column `>= 10`. For
  earlier columns, use the `/*CHECK: ... */` block-comment form.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/declarations.md` (or one of the listed
      secondary docs).
- [ ] The bundle exercises **all major declaration families** named
      in `## Nodes`: variable (`VarDecl`, `LetDecl`, `ParamDecl`),
      aggregate (`StructDecl`, `ClassDecl`, `EnumDecl`,
      `InterfaceDecl`, `ExtensionDecl`), type alias (`TypeDefDecl`,
      `TypeAliasDecl`), callable (`FuncDecl`, `ConstructorDecl`,
      `SubscriptDecl`), accessors (`GetterDecl`, `SetterDecl`,
      `RefAccessorDecl`, `PropertyDecl`), generics (`GenericDecl`,
      `GenericTypeParamDecl`, `GenericValueParamDecl`,
      `GenericTypeConstraintDecl`, `AssocTypeDecl`), namespacing
      (`NamespaceDecl`, `ModuleDecl`, `ImportDecl`, `UsingDecl`),
      and the misc family (`EmptyDecl`, `DeclGroup`, `LambdaDecl`,
      `RequireCapabilityDecl`, `FuncAliasDecl`).
- [ ] At least one negative / diagnostic test per family that has a
      natural negative form (let immutability, interface
      requirement missing, generic constraint violated,
      constructor arity, unresolved name in `using`/`import`).
- [ ] No test asserts the C++ class identity of an AST node, the
      name of a private field, or the parent class in the C++
      hierarchy.
- [ ] No test depends on a GPU. `INTERPRET` and diagnostic-only
      directives carry almost the whole bundle.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `README.md` `## Doc gaps observed` is honest. If you wanted
      to test a behavior but the doc only describes the AST shape
      and not the user-facing role, write down which claim the doc
      would need to add.
- [ ] Claims about internal AST shape (parent class in C++,
      private field names, FIDDLE tag, ASTBuilder allocation,
      synthesized-only classes) are recorded under
      `## Untested claims` in `README.md`, not as tests.
