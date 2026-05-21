---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T13:09:57Z
source_commit: 3250005059a2746ebc504a9d3f71ed112f1f2b94
watched_paths_digest: 328973beec1effabd095eb1784b812673fccfa641b71d28f1e7c83a9228ca518
source_doc: docs/llm-generated/ast-reference/declarations.md
source_doc_digest: 8ee031cac40aa40c1d361b29c159ef5a472b28baec129349366fe4ffd78eac99
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/declarations

## Intent

Tests verify the documented user-observable roles of the concrete
`Decl` / `DeclBase` subclasses enumerated in
[`docs/llm-generated/ast-reference/declarations.md`](../../../docs/llm-generated/ast-reference/declarations.md):
`VarDecl`, `LetDecl`, `ParamDecl`, `StructDecl`, `ClassDecl`,
`EnumDecl`, `EnumCaseDecl`, `InterfaceDecl`, `InheritanceDecl`,
`ExtensionDecl`, `TypeDefDecl`, `TypeAliasDecl`, `NamespaceDecl`,
`FuncDecl`, `ConstructorDecl`, `SubscriptDecl`, `PropertyDecl` and the
`AccessorDecl` family (`GetterDecl`, `SetterDecl`, `RefAccessorDecl`),
`GenericDecl`, `GenericTypeParamDecl`, `GenericValueParamDecl`,
`GenericTypeConstraintDecl`, `AssocTypeDecl`, `ImportDecl`,
`UsingDecl`, `EmptyDecl`, `DeclGroup`, `LambdaDecl`, and
`RequireCapabilityDecl`.

The doc is fundamentally about the internal AST shape of these
classes; the user-observable surface is much smaller than the catalog
of fields. Each test therefore picks one declaration kind and writes
the smallest piece of Slang that exercises that kind's documented
role, with overload resolution, type-checking, or diagnostic
emission as the observable. Negative diagnostic tests cover the
declaration-level rejections the documented role implies
(`let` immutability, missing interface requirement, wrong
constructor arity, generic constraint violation, undeclared name,
unknown module, bad capability name).

Internal AST shape claims (parent class in the C++ hierarchy,
private field names, FIDDLE tag, ASTBuilder allocation,
synthesized-only classes that have no user spelling) are recorded
under `## Out of scope` because they are unobservable through any
allowed `slang-test` directive.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                            | Claim (one line)                                                                                                                       | Tests                                                  |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| C-01     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `VarDecl` is an ordinary mutable variable; its value can be assigned and re-read.                                                    | [`vardecl-mutable.slang`](vardecl-mutable.slang), [`structdecl-default-init.slang`](structdecl-default-init.slang) |
| C-02     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `LetDecl` is an immutable `let` variable; assignment through it is rejected.                                                         | [`letdecl-immutable.slang`](letdecl-immutable.slang)                              |
| C-03     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `ParamDecl` carries direction (`in`/`out`/`inout`); callee mutation is visible at the caller only for `inout`/`out`.                 | [`paramdecl-in-out-inout.slang`](paramdecl-in-out-inout.slang)                         |
| C-04     | [#aggtypedecl-structdecl-classdecl-enumdecl](../../../docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl) | A `StructDecl` is the value-type aggregate variant; copying a struct yields independent storage.                                       | [`structdecl-value-type.slang`](structdecl-value-type.slang)                          |
| C-05     | [#aggtypedecl-structdecl-classdecl-enumdecl](../../../docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl) | A `ClassDecl` is the reference-type aggregate variant; `new` constructs a class instance.                                              | [`classdecl-reference-type.slang`](classdecl-reference-type.slang)                       |
| C-06     | [#aggtypedecl-structdecl-classdecl-enumdecl](../../../docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl) | An `AggTypeDecl` is a container — a struct can nest a struct field, and nested member access reaches the inner storage.               | [`structdecl-nested-struct.slang`](structdecl-nested-struct.slang)                       |
| C-07     | [#aggtypedecl-structdecl-classdecl-enumdecl](../../../docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl) | A `StructDecl` is emitted as a struct by each text-emit backend; the struct keyword and field names appear in the output.              | [`structdecl-emit-multitarget.slang`](structdecl-emit-multitarget.slang)                    |
| C-08     | [#enumdecl-and-enumcasedecl](../../../docs/llm-generated/ast-reference/declarations.md#enumdecl-and-enumcasedecl)                                  | An `EnumDecl` declares a tagged-union enum and `EnumCaseDecl` is one named case; case selection picks the appropriate branch.          | [`enumdecl-tagged-union.slang`](enumdecl-tagged-union.slang)                          |
| C-09     | [#enumdecl-and-enumcasedecl](../../../docs/llm-generated/ast-reference/declarations.md#enumdecl-and-enumcasedecl)                                  | An `EnumCaseDecl` carries its tag value; the case compares equal to the integer it was declared with.                                  | [`enumcasedecl-tag-value.slang`](enumcasedecl-tag-value.slang)                         |
| C-10     | [#enumdecl-and-enumcasedecl](../../../docs/llm-generated/ast-reference/declarations.md#enumdecl-and-enumcasedecl)                                  | An `EnumDecl` is an `AggTypeDecl` so it can have interface conformances; an extension adds one and dispatch fires through it.          | [`enumdecl-conformance.slang`](enumdecl-conformance.slang)                           |
| C-11     | [#inheritancedecl](../../../docs/llm-generated/ast-reference/declarations.md#inheritancedecl)                                                      | An `InterfaceDecl` declares an interface and `InheritanceDecl` ties a conforming type to it; interface-typed dispatch fires correctly. | [`interfacedecl-and-inheritance.slang`](interfacedecl-and-inheritance.slang), [`inheritancedecl-base-interface-method.slang`](inheritancedecl-base-interface-method.slang) |
| C-12     | [#requirementdecl-style-nodes-inside-interfacedecl](../../../docs/llm-generated/ast-reference/declarations.md#requirementdecl-style-nodes-inside-interfacedecl) | An interface requirement is whatever Decl is written inside the interface body; a missing required member is rejected.                | [`interfacedecl-missing-requirement-rejected.slang`](interfacedecl-missing-requirement-rejected.slang)     |
| C-13     | [#extensiondecl](../../../docs/llm-generated/ast-reference/declarations.md#extensiondecl)                                                          | An `ExtensionDecl` attaches new members to an existing type; the added member is callable from outside the extension.                  | [`extensiondecl-adds-member.slang`](extensiondecl-adds-member.slang)                      |
| C-14     | [#extensiondecl](../../../docs/llm-generated/ast-reference/declarations.md#extensiondecl)                                                          | An `ExtensionDecl` can add an interface conformance to an existing type; the type then dispatches through the interface.               | [`extensiondecl-adds-interface-conformance.slang`](extensiondecl-adds-interface-conformance.slang)       |
| C-15     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `TypeDefDecl` introduces a typedef-style alias for an existing type; the alias is interchangeable with the original.                 | [`typedefdecl.slang`](typedefdecl.slang)                                    |
| C-16     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `TypeAliasDecl` introduces a modern-syntax alias (`typealias Y = X`); it is interchangeable with the original.                       | [`typealiasdecl.slang`](typealiasdecl.slang)                                  |
| C-17     | [#namespacedecl-moduledecl-filedecl](../../../docs/llm-generated/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl)                  | A `NamespaceDecl` introduces a named scope; members are reached with `N::name` and same-named namespaces in different N stay distinct. | [`namespacedecl-qualified-lookup.slang`](namespacedecl-qualified-lookup.slang)                 |
| C-18     | [#namespacedecl-moduledecl-filedecl](../../../docs/llm-generated/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl)                  | Multiple textual `namespace N { }` blocks with the same name in one module are collapsed into one `NamespaceDecl`.                     | [`namespacedecl-same-name-collapse.slang`](namespacedecl-same-name-collapse.slang)               |
| C-19     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `FuncDecl` is the ordinary function declaration; FuncDecls overload by parameter type.                                              | [`funcdecl-and-overload.slang`](funcdecl-and-overload.slang)                          |
| C-20     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | The modern `func name(...) -> T` form parses as a FuncDecl with the documented arrow-return spelling.                                  | [`funcdecl-trailing-arrow-return.slang`](funcdecl-trailing-arrow-return.slang)                 |
| C-21     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `FuncDecl` member of an aggregate is a method dispatched on the receiver value.                                                      | [`funcdecl-member-method.slang`](funcdecl-member-method.slang)                         |
| C-22     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `ConstructorDecl` (`__init`) initializes the struct; calling the type as a function dispatches to the constructor.                   | [`constructordecl-init.slang`](constructordecl-init.slang), [`constructordecl-synthesized-member-init.slang`](constructordecl-synthesized-member-init.slang) |
| C-23     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `ConstructorDecl`'s signature is enforced; a wrong-arity constructor call is diagnosed.                                              | [`constructordecl-wrong-arity-rejected.slang`](constructordecl-wrong-arity-rejected.slang)           |
| C-24     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `SubscriptDecl` makes the type indexable; `a[i]` invokes the subscript's get accessor.                                               | [`subscriptdecl-get.slang`](subscriptdecl-get.slang)                              |
| C-25     | [#accessordecl-family](../../../docs/llm-generated/ast-reference/declarations.md#accessordecl-family)                                              | A `PropertyDecl` holds `GetterDecl` and `SetterDecl` children; reading and writing fires the respective accessor.                      | [`propertydecl-getter-setter.slang`](propertydecl-getter-setter.slang)                     |
| C-26     | [#accessordecl-family](../../../docs/llm-generated/ast-reference/declarations.md#accessordecl-family)                                              | A `RefAccessorDecl` returns by reference; a property with an explicit `ref` accessor compiles to the HLSL backend.                     | [`refaccessor-property.slang`](refaccessor-property.slang)                           |
| C-27     | [#genericdecl](../../../docs/llm-generated/ast-reference/declarations.md#genericdecl)                                                              | A `GenericDecl` wraps an inner decl with a parameter list; the same inner decl specializes differently at distinct type arguments.     | [`genericdecl-wrapper.slang`](genericdecl-wrapper.slang)                            |
| C-28     | [#genericdecl](../../../docs/llm-generated/ast-reference/declarations.md#genericdecl)                                                              | A `GenericDecl` for a struct lets the inner be specialized; two specializations carry independent storage.                              | [`genericdecl-different-substitutions.slang`](genericdecl-different-substitutions.slang)            |
| C-29     | [#genericdecl](../../../docs/llm-generated/ast-reference/declarations.md#genericdecl)                                                              | Two nested `GenericDecl`s compose: a generic struct field of a generic struct specializes when the outer specializes.                  | [`genericdecl-recursive.slang`](genericdecl-recursive.slang)                          |
| C-30     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `GenericTypeParamDecl` introduces a type parameter usable inside the generic body.                                                   | [`generic-type-param.slang`](generic-type-param.slang)                             |
| C-31     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `GenericValueParamDecl` introduces a compile-time value parameter usable as a constant inside the inner decl.                        | [`generic-value-param.slang`](generic-value-param.slang)                            |
| C-32     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `GenericTypeConstraintDecl` records a `T : I` constraint; a satisfying argument compiles and dispatches through `I`.                 | [`generic-type-constraint.slang`](generic-type-constraint.slang)                        |
| C-33     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `GenericTypeConstraintDecl` is enforced; a non-satisfying type argument is diagnosed at the call site.                               | [`generic-constraint-violation-rejected.slang`](generic-constraint-violation-rejected.slang)          |
| C-34     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | An `AssocTypeDecl` declares an associated type in an interface; the conforming type supplies a `typealias` and dispatch returns it.    | [`assoctypedecl-interface.slang`](assoctypedecl-interface.slang)                        |
| C-35     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `UsingDecl` brings names from another scope into the current one; an unqualified use resolves.                                       | [`usingdecl-brings-name.slang`](usingdecl-brings-name.slang)                          |
| C-36     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `DeclGroup` wraps multiple decls parsed as a single group (`int a, b;`); each name is independently visible.                          | [`declgroup-multi-var.slang`](declgroup-multi-var.slang)                            |
| C-37     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `LambdaDecl` is the closure-struct produced by a lambda expression; the closure is callable through an `IFunc`-constrained helper.   | [`lambdadecl-closure.slang`](lambdadecl-closure.slang)                             |
| C-38     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `RequireCapabilityDecl` is a module-level declaration; the module compiles when the target capability is met.                        | [`requirecapabilitydecl.slang`](requirecapabilitydecl.slang)                          |
| C-39     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A `RequireCapabilityDecl` with an unknown capability name is rejected.                                                                 | [`requirecapabilitydecl-bad-name-rejected.slang`](requirecapabilitydecl-bad-name-rejected.slang)        |
| C-40     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | An `EmptyDecl` is a declaration that exists only to carry modifiers; a bare semicolon at module scope is accepted.                     | [`emptydecl-bare-semicolon.slang`](emptydecl-bare-semicolon.slang)                       |
| C-41     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | An `ImportDecl` makes another module's decls visible; `import glsl;` succeeds.                                                         | [`importdecl-glsl.slang`](importdecl-glsl.slang)                                |
| C-42     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | An `ImportDecl` that names a module that cannot be found is diagnosed.                                                                 | [`importdecl-unknown-module-rejected.slang`](importdecl-unknown-module-rejected.slang)             |
| C-43     | [#nodes](../../../docs/llm-generated/ast-reference/declarations.md#nodes)                                                                         | A reference to a name with no matching `Decl` in scope is diagnosed.                                                                   | [`declref-undeclared-name-rejected.slang`](declref-undeclared-name-rejected.slang)               |

## Tests in this bundle

| File                                                  | Intent     | Doc anchor                                              |
| ----------------------------------------------------- | ---------- | ------------------------------------------------------- |
| [`vardecl-mutable.slang`](vardecl-mutable.slang)                               | functional | `#nodes`                                                |
| [`letdecl-immutable.slang`](letdecl-immutable.slang)                             | negative   | `#nodes`                                                |
| [`paramdecl-in-out-inout.slang`](paramdecl-in-out-inout.slang)                        | functional | `#nodes`                                                |
| [`structdecl-value-type.slang`](structdecl-value-type.slang)                         | functional | `#aggtypedecl-structdecl-classdecl-enumdecl`            |
| [`structdecl-nested-struct.slang`](structdecl-nested-struct.slang)                      | functional | `#aggtypedecl-structdecl-classdecl-enumdecl`            |
| [`structdecl-default-init.slang`](structdecl-default-init.slang)                       | functional | `#nodes`                                                |
| [`structdecl-emit-multitarget.slang`](structdecl-emit-multitarget.slang)                   | functional | `#aggtypedecl-structdecl-classdecl-enumdecl`            |
| [`classdecl-reference-type.slang`](classdecl-reference-type.slang)                      | functional | `#aggtypedecl-structdecl-classdecl-enumdecl`            |
| [`enumdecl-tagged-union.slang`](enumdecl-tagged-union.slang)                         | functional | `#enumdecl-and-enumcasedecl`                            |
| [`enumcasedecl-tag-value.slang`](enumcasedecl-tag-value.slang)                        | functional | `#enumdecl-and-enumcasedecl`                            |
| [`enumdecl-conformance.slang`](enumdecl-conformance.slang)                          | functional | `#enumdecl-and-enumcasedecl`                            |
| [`interfacedecl-and-inheritance.slang`](interfacedecl-and-inheritance.slang)                 | functional | `#inheritancedecl`                                      |
| [`inheritancedecl-base-interface-method.slang`](inheritancedecl-base-interface-method.slang)         | functional | `#inheritancedecl`                                      |
| [`interfacedecl-missing-requirement-rejected.slang`](interfacedecl-missing-requirement-rejected.slang)    | negative   | `#requirementdecl-style-nodes-inside-interfacedecl`     |
| [`extensiondecl-adds-member.slang`](extensiondecl-adds-member.slang)                     | functional | `#extensiondecl`                                        |
| [`extensiondecl-adds-interface-conformance.slang`](extensiondecl-adds-interface-conformance.slang)      | functional | `#extensiondecl`                                        |
| [`typedefdecl.slang`](typedefdecl.slang)                                   | functional | `#nodes`                                                |
| [`typealiasdecl.slang`](typealiasdecl.slang)                                 | functional | `#nodes`                                                |
| [`namespacedecl-qualified-lookup.slang`](namespacedecl-qualified-lookup.slang)                | functional | `#namespacedecl-moduledecl-filedecl`                    |
| [`namespacedecl-same-name-collapse.slang`](namespacedecl-same-name-collapse.slang)              | functional | `#namespacedecl-moduledecl-filedecl`                    |
| [`funcdecl-and-overload.slang`](funcdecl-and-overload.slang)                         | functional | `#nodes`                                                |
| [`funcdecl-trailing-arrow-return.slang`](funcdecl-trailing-arrow-return.slang)                | functional | `#nodes`                                                |
| [`funcdecl-member-method.slang`](funcdecl-member-method.slang)                        | functional | `#nodes`                                                |
| [`constructordecl-init.slang`](constructordecl-init.slang)                          | functional | `#nodes`                                                |
| [`constructordecl-synthesized-member-init.slang`](constructordecl-synthesized-member-init.slang)       | functional | `#nodes`                                                |
| [`constructordecl-wrong-arity-rejected.slang`](constructordecl-wrong-arity-rejected.slang)          | negative   | `#nodes`                                                |
| [`subscriptdecl-get.slang`](subscriptdecl-get.slang)                             | functional | `#nodes`                                                |
| [`propertydecl-getter-setter.slang`](propertydecl-getter-setter.slang)                    | functional | `#accessordecl-family`                                  |
| [`refaccessor-property.slang`](refaccessor-property.slang)                          | functional | `#accessordecl-family`                                  |
| [`genericdecl-wrapper.slang`](genericdecl-wrapper.slang)                           | functional | `#genericdecl`                                          |
| [`genericdecl-different-substitutions.slang`](genericdecl-different-substitutions.slang)           | functional | `#genericdecl`                                          |
| [`genericdecl-recursive.slang`](genericdecl-recursive.slang)                         | functional | `#genericdecl`                                          |
| [`generic-type-param.slang`](generic-type-param.slang)                            | functional | `#nodes`                                                |
| [`generic-value-param.slang`](generic-value-param.slang)                           | functional | `#nodes`                                                |
| [`generic-type-constraint.slang`](generic-type-constraint.slang)                       | functional | `#nodes`                                                |
| [`generic-constraint-violation-rejected.slang`](generic-constraint-violation-rejected.slang)         | negative   | `#nodes`                                                |
| [`assoctypedecl-interface.slang`](assoctypedecl-interface.slang)                       | functional | `#nodes`                                                |
| [`usingdecl-brings-name.slang`](usingdecl-brings-name.slang)                         | functional | `#nodes`                                                |
| [`declgroup-multi-var.slang`](declgroup-multi-var.slang)                           | functional | `#nodes`                                                |
| [`lambdadecl-closure.slang`](lambdadecl-closure.slang)                            | functional | `#nodes`                                                |
| [`requirecapabilitydecl.slang`](requirecapabilitydecl.slang)                         | functional | `#nodes`                                                |
| [`requirecapabilitydecl-bad-name-rejected.slang`](requirecapabilitydecl-bad-name-rejected.slang)       | negative   | `#nodes`                                                |
| [`emptydecl-bare-semicolon.slang`](emptydecl-bare-semicolon.slang)                      | functional | `#nodes`                                                |
| [`importdecl-glsl.slang`](importdecl-glsl.slang)                               | functional | `#nodes`                                                |
| [`importdecl-unknown-module-rejected.slang`](importdecl-unknown-module-rejected.slang)            | negative   | `#nodes`                                                |
| [`declref-undeclared-name-rejected.slang`](declref-undeclared-name-rejected.slang)              | negative   | `#nodes`                                                |

## Doc gaps observed

- The doc lists `FuncAliasDecl` as "function alias / re-export of an
  existing callable" with a grammar link to `[func alias]`. In
  practice `FuncAliasDecl` is synthesized by the checker for forms
  like forward-mode differentiation rather than spelled in user
  code; the surface syntax `func aliasedName = original;` is not
  accepted by `slangc`. Either the doc's grammar pointer should be
  removed, or the doc should clarify that `FuncAliasDecl` is
  checker-synthesized.
- The doc enumerates `SynthesizedStructDecl`, `SynthesizedFuncDecl`,
  `InterfaceDefaultImplDecl`, `ThisTypeDecl`, `ThisTypeConstraintDecl`,
  and `UnresolvedDecl` as concrete classes, but each is internal —
  there is no user spelling and no externally observable behavior
  distinct from the surrounding decl. A one-line note "synthesized
  by the checker, no user spelling" would let an agent skip them
  without doubt.
- The doc mentions `ModernParamDecl` ("modern-syntax parameter;
  immutable unless `out`/`inout`") but does not specify which
  parameter spelling produces `ModernParamDecl` vs. `ParamDecl`. The
  parameter-direction test here covers the direction half of the
  claim; the `ModernParamDecl`-vs-`ParamDecl` distinction would need
  a doc-level pointer to the modern parameter grammar.
- The doc says `EnumDecl::tagType` carries the underlying integer
  type but does not state a user-facing claim about how the tag type
  affects, e.g., `int(c)` conversion. The tag-value test here uses
  the default tag type and so does not exercise the alternative.
- The doc mentions `EnumDecl` exhaustive matching is part of the
  AggTypeDecl machinery; the doc does not state whether a
  `switch` without `default` over an enum is diagnosed. (In our
  experimentation `switch` lowering in the interpreter is the
  blocker; a stmt-tests page would be a better home.) The doc
  could spell this out so a future agent can either anchor a
  diagnostic test here or defer to `ast-reference/statements`.
- `RequireCapabilityDecl`'s observable behavior on a Slang
  unknown-capability name (the C-39 test) emits a "capability does
  not exist" error; the source-doc text only says "expressed as a
  decl so it can be exported". A one-line claim about the
  diagnostic would make the negative test directly anchorable.
- The doc lists `GLSLInterfaceBlockDecl` as "GLSL-style interface
  block (uniform/buffer/in/out)". The user-observable surface is
  GLSL-only; this bundle does not exercise it because the
  interpreter cannot model GLSL interface blocks. A pointer from
  the doc to a GLSL emit-side test would resolve this.

## Out of scope

The doc is overwhelmingly about internal AST shape. The following
claim families are not observable through `slangc` / `slang-test`
directives that this bundle can run; they are recorded here rather
than tested.

- The C++ parent class of any concrete `Decl` (e.g. that
  `LetDecl extends VarDecl`, that `ConstructorDecl` extends
  `FunctionDeclBase`, that `GenericTypeParamDecl` extends
  `GenericTypeParamDeclBase`). Only the user-observable behavior
  is testable.
- Field names listed in the `## Nodes` table (e.g.
  `VarDecl::type: TypeExp`, `VarDecl::initExpr: Expr*`,
  `GenericDecl::inner: Decl*`, `ExtensionDecl::targetType: TypeExp`,
  `EnumCaseDecl::tagVal: IntVal*`,
  `InheritanceDecl::witnessTable: RefPtr<WitnessTable>`,
  `ConstructorDecl::m_flavor: int`, `LambdaDecl::funcDecl`,
  `ModuleDecl::module: Module*`).
- The grammar production a parser callback is named (e.g.
  `parseAggTypeDecl`, `parseRequireCapabilityDecl`,
  `parseGenericDecl`).
- The `## Family hierarchy` mermaid diagram as a graph: abstract
  intermediates (`ContainerDecl`, `CallableDecl`, `AggTypeDeclBase`,
  `FunctionDeclBase`, `VarDeclBase`, `SimpleTypeDecl`,
  `TypeConstraintDecl`, `NamespaceDeclBase`,
  `GenericTypeParamDeclBase`, `FileReferenceDeclBase`,
  `IncludeDeclBase`) carry no FIDDLE concrete tag and produce no
  user spelling of their own.
- Synthesized-only concrete classes that have no user-visible
  spelling: `SynthesizedStructDecl`, `SynthesizedFuncDecl`,
  `UnresolvedDecl`, `InterfaceDefaultImplDecl`, `ThisTypeDecl`,
  `ThisTypeConstraintDecl`. Their existence shows up only through
  the surrounding feature (interface dispatch, tuple types,
  default-impl resolution) and the surrounding feature is what we
  test.
- The `SyntaxDecl` model of "keyword binding produced by `__syntax`"
  is the subject of
  `tests-agentic/syntax-reference/keywords-and-builtins/`, which
  exercises keyword recognition from the surface-syntax side. The
  AST-shape claim ("the keyword maps to a parse callback") is not a
  separate user-observable.
- `AttributeDecl::syntaxClass` is a reflection handle for an
  attribute class; the attribute's effect is observable through
  attribute-bearing tests in other bundles, but the attribute-class
  identity itself is not.
- `FuncAliasDecl` (see doc gap above): no user spelling.
- `EmptyDecl::layout(...) in;` form: the doc cites GLSL-specific
  syntax; we exercise the simpler bare-`;` form that the parser
  also produces an `EmptyDecl` for.
- `ModuleDeclarationDecl` (the `module M;` opener): structuring a
  multi-file module requires more than one source file and so is
  outside the single-file `slang-test` surface this bundle can
  exercise.
- `IncludeDecl` / `ImplementingDecl` / `__include` /
  `__implementing`: same reason — they reference companion files.
- `GLSLInterfaceBlockDecl`: GLSL-only surface; not exercised here
  (see doc gap above).
- `Decl::checkState` / `Decl::inferredCapabilityRequirements`:
  internal checking-phase fields. Capability gating is the subject
  of `pipeline/03-semantic-check`'s capability tests; this bundle
  covers the user-spelling-side `RequireCapabilityDecl` only.
