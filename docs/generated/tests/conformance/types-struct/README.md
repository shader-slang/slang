---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:02:08+00:00
source_commit: 9c1a6a00ef413932805da5b813465a7a9d517fb9
watched_paths_digest: b23f452f4ef91d00984635cf6a619bc7faedb364aafb6094ee13933a44b66785
source_doc: docs/language-reference/types-struct.md
source_doc_digest: 192c9bfea8b11ea48ba1b5bdeec1cebc30aa5cbb8419686efe655f878f50edce
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/types-struct

## Intent

Tests verify struct-type claims in the **language reference** at
[`docs/language-reference/types-struct.md`](../../../../language-reference/types-struct.md).
Coverage strategy: one functional test (INTERPRET or emit) per claim, expanded along documented
dimensions (member kinds, visibility, constructor overloads, accessor forms, subscript
arities, call-operator overloads, memory layout, generic structs, interface conformance,
nested types, and `This` type keyword). Negative tests cover write-to-readonly-member and
const-member-in-constructor. Static data members are tested via HLSL emission only because
the slangi VM bytecode emitter does not support module-scope global instructions.

## Claims

### Syntax and forms

- **C1** A struct declared with the no-body form specifies the existence of a structure type,
  enabling use in type expressions before the full definition.
- **C2** The with-members form defines the structure type with a layout and a list of non-layout members.
- **C3** Struct names are used as identifiers; when the identifier is omitted the struct is anonymous
  and is assigned an unspecified unique name.
- **C4** Anonymous structs are used in inline variable declarations (`struct { int a; } obj;`).
- **C5** `extern struct` and `export struct` forms exist for link-time type definitions.

### Description — member kinds

- **C6** A non-static data member is declared as a variable without `static`; its storage is
  allocated per instance as part of the struct.
- **C7** A static data member is declared with `static`; its storage is allocated from global storage.
- **C8** `static const` data members must have a default initializer.
- **C9** A nested type enclosed in the struct body is accessed via the outer struct name.
- **C10** Non-static data members are allocated sequentially within the struct.
- **C11** A struct may conform to one or more interface types.
- **C12** Members may be declared with access specifiers `public`, `internal`, or `private`; the
  default is `internal`.

### Object (`#object`)

- **C13** An instance (object) consists of all non-static data members.
- **C14** Data members may be initialized using an initializer list or a constructor.

### Constructors (`#constructor`)

- **C15** When a user-provided `__init` constructor is defined, it is executed on object instantiation.
- **C16** More than one `__init` may be defined; overload resolution selects the most appropriate one.
- **C17** When no initializer is provided, the no-parameter constructor is invoked.
- **C18** If a non-static data member is not initialized by the constructor, it has an undefined state.
- **C19** `const` data members cannot be initialized by the constructor.
- **C20** When no user-provided constructor exists, aggregate initialization is performed.
- **C21** With aggregate initialization, if the initializer list does not contain enough values,
  remaining members are default-initialized.

### Default initializers

- **C22** When an object is initialized using an initializer list, a member's default initializer
  supplies its value when the list does not provide one.
- **C23** When a constructor is used, a non-`const` member's default initializer specifies the
  initial value which the constructor may then override.

### Static Member Functions (`#static-member-function`)

- **C24** A static member function is invoked without an object.
- **C25** A static member function may access only static data members.

### Non-static Member Functions (`#nonstatic-member-function`)

- **C26** A non-static member function has a hidden `this` parameter; `this.member` and bare
  `member` both access the same field.
- **C27** By default, member functions have read-only access to non-static data members.
- **C28** `[mutating]` on a member function grants write access to non-static data members.
- **C29** Writing to a non-static member in a non-`[mutating]` function is a compile-time error (E30011).

### Properties (`#property`)

- **C30** A `property` with `get` and `set` accessors routes reads to `get` and writes to `set`.
- **C31** A read-only property has only a `get` accessor.
- **C32** When the `set` parameter is omitted, an implicit `newValue` parameter of the property's
  type is available in the set body.
- **C33** An explicit parameter name may be given in the `set` accessor declaration.

### Accessing Members and Nested Types (`#accessing-members-and-nested-types`)

- **C34** Static and non-static members and nested types are accessed via the dot `.` operator.
- **C35** The `::` scope operator is deprecated for static member access; `.` is recommended.

### Subscript Operator (`#subscript-op`)

- **C36** `__subscript` with `get` and `set` accessors enables `[]` syntax; read invokes `get`,
  write invokes `set`.
- **C37** Multiple `__subscript` overloads with different signatures are resolved by overload resolution.
- **C38** A subscript may have zero parameters.

### Function Call Operator (`#function-call-op`)

- **C39** `operator ()` lets a struct instance be called as if it were a function.
- **C40** Multiple `operator ()` overloads with different signatures are resolved by overload resolution.

### Memory Layout (`#natural-layout`, `#c-style-layout`, `#d3d-constant-buffer-layout`)

- **C41** In natural layout, non-static data members are emitted in declaration order.
- **C42** Natural layout: struct size is not tail-padded (C-style layout adds tail-padding).
- **C43** D3D constant buffer layout has a minimum alignment of 16 and an improper-straddle rule.

### `This` type keyword

- **C44** Inside a struct body, `This` refers to the enclosing struct type itself.

## Functional coverage

| Claim                                                                                                                       | Intent     | Anchor                                                                                                                   | Tests                                                                                                                                                      |
| --------------------------------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1 — No-body struct declaration allows use in type expressions before full definition.                                      | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-no-body-decl.slang`](struct-no-body-decl.slang)                                                                                                   |
| C3 — Named struct declared with `struct Foo { ... }` uses `Foo` as the type name; fields accessed via dot.                  | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-basic-fields-functional.slang`](struct-basic-fields-functional.slang)                                                                             |
| C4 — Anonymous struct `struct { int a; } obj;` allows dot-access to its fields.                                             | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-anonymous.slang`](struct-anonymous.slang)                                                                                                         |
| C6 — Non-static fields are read and written via the dot operator on an instance.                                            | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-basic-fields-functional.slang`](struct-basic-fields-functional.slang), [`struct-basic-fields-emission.slang`](struct-basic-fields-emission.slang) |
| C7 — Static data member allocated from global storage; emitted as a module-scope static in HLSL.                            | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-static-member-functional.slang`](struct-static-member-functional.slang)                                                                           |
| C8 — `static const` data member with a default initializer is accessible via the type name and folds to the constant value. | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-static-const-initializer.slang`](struct-static-const-initializer.slang)                                                                           |
| C9 — A nested type is accessed via the outer struct name with dot notation.                                                 | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-nested-type.slang`](struct-nested-type.slang)                                                                                                     |
| C10 — Non-static data members are allocated sequentially; emitted in declaration order in HLSL.                             | functional | [#natural-layout](../../../../language-reference/types-struct.md#natural-layout)                                         | [`struct-natural-layout-emission.slang`](struct-natural-layout-emission.slang)                                                                             |
| C11 — A struct conforming to an interface can be passed to a generic constrained on that interface.                         | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-interface-conformance.slang`](struct-interface-conformance.slang)                                                                                 |
| C12 — Members declared `public` are accessible; struct can be declared `public` to allow `public` members.                  | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-access-visibility-default-internal.slang`](struct-access-visibility-default-internal.slang)                                                       |
| C13/C14 — Instance fields are per-object; initializer list sets field values at construction.                               | functional | [#object](../../../../language-reference/types-struct.md#object)                                                         | [`struct-aggregate-init.slang`](struct-aggregate-init.slang)                                                                                               |
| C15 — User-defined `__init()` runs on object instantiation; fields have the constructor-assigned values.                    | functional | [#constructor](../../../../language-reference/types-struct.md#constructor)                                               | [`struct-constructor-no-param.slang`](struct-constructor-no-param.slang)                                                                                   |
| C16/C17 — Multiple `__init` constructors; overload resolution selects the right one by argument count.                      | functional | [#constructor](../../../../language-reference/types-struct.md#constructor)                                               | [`struct-constructor-overload.slang`](struct-constructor-overload.slang)                                                                                   |
| C19 — Assigning to a `const` data member inside `__init` is a compile error (E30011).                                       | negative   | [#constructor](../../../../language-reference/types-struct.md#constructor)                                               | [`struct-constructor-no-const-member.slang`](struct-constructor-no-const-member.slang)                                                                     |
| C20/C21 — Aggregate initialization fills fields in order; unspecified trailing fields are default-initialized.              | functional | [#constructor](../../../../language-reference/types-struct.md#constructor)                                               | [`struct-aggregate-init.slang`](struct-aggregate-init.slang)                                                                                               |
| C22 — In initializer-list initialization, a default initializer supplies the value for a field not given in the list.       | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-default-field-initializer.slang`](struct-default-field-initializer.slang)                                                                         |
| C23 — A constructor may override a non-`const` field's default initializer by assigning explicitly in the body.             | functional | [#description](../../../../language-reference/types-struct.md#description)                                               | [`struct-default-initializer-constructor-override.slang`](struct-default-initializer-constructor-override.slang)                                           |
| C24/C25 — Static member function invoked without an object; emits as a free function in HLSL.                               | functional | [#static-member-function](../../../../language-reference/types-struct.md#static-member-function)                         | [`struct-static-member-function.slang`](struct-static-member-function.slang)                                                                               |
| C26 — `this.member` and bare `member` both access the same field inside a non-static member function.                       | functional | [#nonstatic-member-function](../../../../language-reference/types-struct.md#nonstatic-member-function)                   | [`struct-this-implicit-optional.slang`](struct-this-implicit-optional.slang)                                                                               |
| C27 — Non-static member function reads fields without `[mutating]`; return value is correct.                                | functional | [#nonstatic-member-function](../../../../language-reference/types-struct.md#nonstatic-member-function)                   | [`struct-member-function-readonly.slang`](struct-member-function-readonly.slang)                                                                           |
| C28 — `[mutating]` member function writes to a non-static field; the mutation is observable on the instance.                | functional | [#nonstatic-member-function](../../../../language-reference/types-struct.md#nonstatic-member-function)                   | [`struct-mutating-member-function.slang`](struct-mutating-member-function.slang)                                                                           |
| C29 — Writing to a non-static member in a non-`[mutating]` function emits error E30011 (not an l-value).                    | negative   | [#nonstatic-member-function](../../../../language-reference/types-struct.md#nonstatic-member-function)                   | [`struct-mutating-rejected-without-attribute.slang`](struct-mutating-rejected-without-attribute.slang)                                                     |
| C30 — `property` with `get` and `set`: reading routes to `get`, writing routes to `set` with implicit `newValue`.           | functional | [#property](../../../../language-reference/types-struct.md#property)                                                     | [`struct-property-get-set.slang`](struct-property-get-set.slang)                                                                                           |
| C31 — A `property` with only a `get` accessor is read-only; the getter emits correctly in HLSL.                             | functional | [#property](../../../../language-reference/types-struct.md#property)                                                     | [`struct-property-readonly.slang`](struct-property-readonly.slang)                                                                                         |
| C33 — An explicit parameter name in the `set` accessor is used in the set body instead of `newValue`.                       | functional | [#property](../../../../language-reference/types-struct.md#property)                                                     | [`struct-property-explicit-param.slang`](struct-property-explicit-param.slang)                                                                             |
| C34 — Static and non-static members, nested types, and member functions are all accessed via dot `.`.                       | functional | [#accessing-members-and-nested-types](../../../../language-reference/types-struct.md#accessing-members-and-nested-types) | [`struct-dot-member-access.slang`](struct-dot-member-access.slang)                                                                                         |
| C36 — `__subscript` with `get`/`set` enables `[]` read/write on struct instances.                                           | functional | [#subscript-op](../../../../language-reference/types-struct.md#subscript-op)                                             | [`struct-subscript-op-functional.slang`](struct-subscript-op-functional.slang)                                                                             |
| C37/C38 — Multiple `__subscript` overloads including a zero-param form; overload resolution dispatches by arity.            | functional | [#subscript-op](../../../../language-reference/types-struct.md#subscript-op)                                             | [`struct-subscript-multi-param.slang`](struct-subscript-multi-param.slang)                                                                                 |
| C39/C40 — `operator ()` allows calling an object like a function; multiple overloads resolved by parameter count.           | functional | [#function-call-op](../../../../language-reference/types-struct.md#function-call-op)                                     | [`struct-call-op-functional.slang`](struct-call-op-functional.slang)                                                                                       |
| C41 — Natural layout: non-static fields emitted in declaration order in HLSL struct type.                                   | functional | [#natural-layout](../../../../language-reference/types-struct.md#natural-layout)                                         | [`struct-natural-layout-emission.slang`](struct-natural-layout-emission.slang)                                                                             |
| C44 — `This` inside a struct body refers to the enclosing struct type; usable as return type in a member function.          | functional | [#d3d-constant-buffer-layout](../../../../language-reference/types-struct.md#d3d-constant-buffer-layout)                 | [`struct-This-type-keyword.slang`](struct-This-type-keyword.slang)                                                                                         |
| Struct with generic type parameters `<T>` instantiated at int and float; fields typed by T.                                 | functional | [#syntax](../../../../language-reference/types-struct.md#syntax)                                                         | [`struct-generic.slang`](struct-generic.slang)                                                                                                             |
| Full doc example with static/non-static members, functions, nested struct emits valid HLSL.                                 | functional | [#accessing-members-and-nested-types](../../../../language-reference/types-struct.md#accessing-members-and-nested-types) | [`struct-members-full-example-emission.slang`](struct-members-full-example-emission.slang)                                                                 |

## Untested claims

| Claim                                                                                      | Reason                | Anchor                                                                                                                   | Why untested                                                                                                                                   |
| ------------------------------------------------------------------------------------------ | --------------------- | ------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| C2 — link-time `extern struct` form declares a struct defined in another module.           | needs-multi-file-test | [#description](../../../../language-reference/types-struct.md#description)                                               | Requires module linking across two `.slang` files and a `-module` output; not testable with a single `//TEST` directive.                       |
| C5 — `export struct` form exports a struct with a type alias to another module.            | needs-multi-file-test | [#description](../../../../language-reference/types-struct.md#description)                                               | Requires multi-file module compilation and linking.                                                                                            |
| C18 — A non-static data member not initialized by the constructor has an undefined state.  | (unclassified)        | [#constructor](../../../../language-reference/types-struct.md#constructor)                                               | Undefined behavior cannot be asserted; the compiler is correct to allow any value. No testable claim.                                          |
| C32 — `set` parameter is optional; implicit `newValue` parameter is provided when omitted. | out-of-bundle         | [#property](../../../../language-reference/types-struct.md#property)                                                     | Covered by [`struct-property-get-set.slang`](struct-property-get-set.slang) which uses implicit `newValue`.                                    |
| C35 — `::` is deprecated for static member access; compiler should warn or accept.         | deprecated            | [#accessing-members-and-nested-types](../../../../language-reference/types-struct.md#accessing-members-and-nested-types) | The doc warns `::` is deprecated, but the deprecation warning behavior (warn vs accept) is not specified. No testable claim on the error code. |
| C42 — Natural layout has no tail-padding; C-style layout rounds size up to alignment.      | needs-unit-test       | [#c-style-layout](../../../../language-reference/types-struct.md#c-style-layout)                                         | Not observable from the slangc CLI; requires the reflection API to query struct size and alignment.                                            |
| C43 — D3D constant buffer layout has minimum alignment 16 and the improper-straddle rule.  | needs-unit-test       | [#d3d-constant-buffer-layout](../../../../language-reference/types-struct.md#d3d-constant-buffer-layout)                 | Not observable from the slangc CLI without reflection. Requires a cbuffer layout query or `sizeof` operator not available in test shaders.     |

## Doc gaps observed

| Anchor                                                                                                   | Kind            | Gap                                                                                                                                                                                                                                                                                                                                                                                   | Suggested addition                                                                                                                                                                                                       |
| -------------------------------------------------------------------------------------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [#d3d-constant-buffer-layout](../../../../language-reference/types-struct.md#d3d-constant-buffer-layout) | missing-surface | The `This` keyword section appears at the very end of the document without a heading anchor (the section heading is "This Type" but the preceding major heading is `# Memory Layout`), making it impossible to cite by a stable `#this-type` anchor. The lint tool cannot validate the doc_ref anchor fragment.                                                                       | Add an explicit `{#this-type}` heading anchor to the "This Type" subsection so bundles can anchor `doc_ref` to it precisely.                                                                                             |
| [#constructor](../../../../language-reference/types-struct.md#constructor)                               | missing-example | The doc states that when no initializer list is provided and there is no user-provided constructor, the class is instantiated in an undefined state (Remark 1). No minimal example shows this for a struct that has fields with user-provided constructors at the member level (the sub-struct `TestField` case). The remark says the outer struct's fields are undefined regardless. | Add a standalone code snippet that isolates the case: outer struct with no constructor and an inner field type that has `__init`; add a comment "// obj.f is in undefined state despite TestField having a constructor." |
